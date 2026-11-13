#include "exception.h"
#include "gic.h"
#include "timer.h"
#include "uart.h"

#include <stdint.h>

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

    exception_vectors_install();
    uart_puts("\nException vectors: installed\n");

    gic_init();
    uart_puts("GICv2: Group 1 CPU/distributor enabled\n");

    timer_init();
    uart_puts("Generic Timer: 10 Hz configured\n");
    uart_puts("Enabling IRQs; waiting for timer interrupts...\n");

    __asm__ volatile("dsb sy\nisb" ::: "memory");
    enable_irq();

    unsigned long reported = 0;
    for (;;) {
        unsigned long now = timer_ticks();
        while (reported < now) {
            ++reported;
            uart_puts("timer: tick ");
            uart_hex(reported);
            uart_puts("\n");
        }
        __asm__ volatile("wfe");
    }
}
