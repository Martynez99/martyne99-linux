#include "exception.h"
#include "gic.h"
#include "timer.h"
#include "uart.h"

#include <stdint.h>

#define GICD_BASE 0x08000000UL
#define GICC_BASE 0x08010000UL
#define MMIO32(addr) (*(volatile uint32_t *)(addr))

extern char __kernel_start[];
extern char __kernel_end[];

static unsigned long read_current_el(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(value));
    return (value >> 2) & 3UL;
}

static unsigned long read_cntfrq(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(value));
    return value;
}

static unsigned long read_cntpct(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, cntpct_el0" : "=r"(value));
    return value;
}

static unsigned long read_cntp_ctl(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, cntp_ctl_el0" : "=r"(value));
    return value;
}

static unsigned long read_cntp_tval(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, cntp_tval_el0" : "=r"(value));
    return value;
}

static void print_reg(const char *name, unsigned long value)
{
    uart_puts(name);
    uart_hex(value);
    uart_puts("\n");
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

    /* Let one timer deadline pass with IRQs still masked. */
    unsigned long frequency = read_cntfrq();
    unsigned long start = read_cntpct();
    while ((read_cntpct() - start) < (frequency / 4UL))
        __asm__ volatile("nop");

    unsigned long timer_ctl = read_cntp_ctl();
    print_reg("CNTFRQ_EL0: ", frequency);
    print_reg("CNTP_CTL_EL0: ", timer_ctl);
    print_reg("CNTP_TVAL_EL0: ", read_cntp_tval());
    print_reg("GICD_CTLR: ", MMIO32(GICD_BASE + 0x000));
    print_reg("GICD_IGROUPR0: ", MMIO32(GICD_BASE + 0x080));
    print_reg("GICD_ISENABLER0: ", MMIO32(GICD_BASE + 0x100));
    print_reg("GICC_CTLR: ", MMIO32(GICC_BASE + 0x000));
    print_reg("GICC_PMR: ", MMIO32(GICC_BASE + 0x004));
    if (timer_ctl & 4UL)
        uart_puts("Timer diagnostic: deadline expired\n");
    else
        uart_puts("Timer diagnostic: deadline NOT expired\n");

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
