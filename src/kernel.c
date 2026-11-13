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

void kernel_main(void)
{
    uart_init();

    uart_puts("\n\n");
    uart_puts("  __  __          _____ _______   _   _ ______   _      _____ _   _ _   _ X\n");
    uart_puts(" |  \/  |   /\   |  __ \__   __| | \ | |  ____| | |    |_   _| \ | | | | |\n");
    uart_puts(" | \  / |  /  \  | |__) | | |    |  \| | |__    | |      | | |  \| | | | |\n");
    uart_puts(" | |\/| | / /\ \ |  _  /  | |    | . ` |  __|   | |      | | | . ` | | | |\n");
    uart_puts(" | |  | |/ ____ \| | \ \  | |    | |\  | |____  | |____ _| |_| |\  | |_| |\n");
    uart_puts(" |_|  |_/_/    \_\_|  \_\ |_|    |_| \_|______| |______|_____|_| \_|\___/\n\n");

    uart_puts("MARTYNE LINUX: M1 boot OK\n");
    uart_puts("Architecture : AArch64\n");
    uart_puts("Exception EL : EL");
    uart_putc((char)('0' + read_current_el()));
    uart_puts("\nKernel start: ");
    uart_hex((unsigned long)(uintptr_t)__kernel_start);
    uart_puts("\nKernel end  : ");
    uart_hex((unsigned long)(uintptr_t)__kernel_end);
    uart_puts("\nStatus       : MMU off, interrupts masked\n");
    uart_puts("\nM1 foundation is alive. Next: exception vectors, GIC, and timer.\n");

    for (;;)
        __asm__ volatile("wfe");
}
