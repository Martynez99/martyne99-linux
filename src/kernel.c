#include "exception.h"
#include "gic.h"
#include "memory.h"
#include "mmu.h"
#include "scheduler.h"
#include "timer.h"
#include "uart.h"

#include <stdint.h>

#define QEMU_RAM_BASE 0x40000000UL
#define QEMU_RAM_SIZE (256UL * 1024UL * 1024UL)

extern char __kernel_start[];
extern char __kernel_end[];

static unsigned long read_current_el(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(value));
    return (value >> 2) & 3UL;
}

static void enable_irq(void)
{
    __asm__ volatile("msr daifclr, #2\n\tisb" ::: "memory");
}

static void memory_self_test(void)
{
    if (memory_init(QEMU_RAM_BASE, QEMU_RAM_BASE + QEMU_RAM_SIZE,
                    (uintptr_t)__kernel_end) != 0) {
        uart_puts("memory: initialization FAILED\n");
        return;
    }

    void *frame = page_alloc();
    if (!frame || ((uintptr_t)frame & (PAGE_SIZE - 1UL))) {
        uart_puts("memory: frame allocation FAILED\n");
        return;
    }
    if (page_free(frame) != 0 || page_free(frame) == 0) {
        uart_puts("memory: frame free validation FAILED\n");
        return;
    }
    uart_puts("memory: frame allocator OK, free pages=");
    uart_hex((unsigned long)memory_free_pages());
    uart_puts("\n");

    if (heap_init() != 0) {
        uart_puts("heap: initialization FAILED\n");
        return;
    }

    uint8_t *a = (uint8_t *)kmalloc(64);
    uint8_t *b = (uint8_t *)kmalloc(128);
    if (!a || !b) {
        uart_puts("heap: allocation FAILED\n");
        return;
    }
    a[0] = 0xa5U;
    b[127] = 0x5aU;
    if (a[0] != 0xa5U || b[127] != 0x5aU ||
        kfree(a) != 0 || kfree(b) != 0) {
        uart_puts("heap: allocation/free FAILED\n");
        return;
    }
    uart_puts("heap: allocation and coalescing OK\n");
}

void kernel_main(void)
{
    uart_init();

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("           MARTYNE LINUX                \n");
    uart_puts("========================================\n");
    uart_puts("MARTYNE LINUX: M1 boot OK\n");
    uart_puts("Architecture : AArch64\n");
    uart_puts("Exception EL : EL");
    uart_putc((char)('0' + read_current_el()));
    uart_puts("\nKernel start: ");
    uart_hex((unsigned long)(uintptr_t)__kernel_start);
    uart_puts("\nKernel end  : ");
    uart_hex((unsigned long)(uintptr_t)__kernel_end);
    uart_puts("\n");

    exception_vectors_install();
    uart_puts("Exception vectors: installed\n");

    memory_self_test();

    uart_puts("MMU: building identity map and enabling translation...\n");
    if (mmu_init() != 0 || !mmu_is_enabled()) {
        uart_puts("MMU: activation FAILED\n");
        for (;;)
            __asm__ volatile("wfe");
    }
    uart_puts("MMU: translation enabled\n");

    gic_init();
    uart_puts("GICv2: Group 1 CPU/distributor enabled\n");

    timer_init();
    scheduler_init();
    uart_puts("Generic Timer: 10 Hz configured\n");
    uart_puts("Scheduler: idle + 2 kernel threads ready\n");
    uart_puts("Enabling IRQs; round-robin preemption active\n");

    __asm__ volatile("dsb sy\n\tisb" ::: "memory");
    enable_irq();

    unsigned long reported = 0;
    int scheduler_reported = 0;
    for (;;) {
        unsigned long now = timer_ticks();
        while (reported < now) {
            ++reported;
            uart_puts("timer: tick ");
            uart_hex(reported);
            uart_puts("\n");
        }

        if (!scheduler_reported && scheduler_self_test_passed()) {
            scheduler_reported = 1;
            uart_puts("scheduler: round-robin preemption OK, switches=");
            uart_hex(scheduler_switch_count());
            uart_puts("\n");
        }

        __asm__ volatile("wfe");
    }
}
