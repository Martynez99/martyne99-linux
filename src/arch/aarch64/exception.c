#include "exception.h"
#include "gic.h"
#include "uart.h"

static void exception_panic(unsigned long esr, unsigned long far,
                            unsigned long elr)
{
    uart_puts("\nFATAL: synchronous exception\nESR: ");
    uart_hex(esr);
    uart_puts("\nFAR: ");
    uart_hex(far);
    uart_puts("\nELR: ");
    uart_hex(elr);
    uart_puts("\nSystem halted.\n");
    for (;;)
        __asm__ volatile("wfe");
}

void exception_vectors_install(void)
{
    extern char exception_vectors[];
    __asm__ volatile("msr vbar_el1, %0; isb" :: "r"(exception_vectors) : "memory");
}

void exception_dispatch(unsigned long vector, unsigned long esr,
                        unsigned long far, unsigned long elr)
{
    if (vector == 1) {
        gic_handle_irq();
        return;
    }

    exception_panic(esr, far, elr);
}
