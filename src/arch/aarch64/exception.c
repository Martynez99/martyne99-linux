#include "exception.h"
#include "gic.h"
#include "scheduler.h"
#include "timer.h"
#include "uart.h"

#define ARM_GENERIC_TIMER_PPI 30U

static __attribute__((noreturn))
void exception_panic(unsigned long esr, unsigned long far,
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
    __asm__ volatile("msr vbar_el1, %0; isb"
                     :: "r"(exception_vectors) : "memory");
}

exception_frame_t *exception_dispatch(unsigned long vector,
                                      unsigned long esr,
                                      unsigned long far,
                                      unsigned long elr,
                                      exception_frame_t *frame)
{
    if (vector == 1UL) {
        uint32_t interrupt_id = gic_handle_irq();
        if (interrupt_id == ARM_GENERIC_TIMER_PPI)
            return scheduler_on_timer(frame);
        return frame;
    }

    exception_panic(esr, far, elr);
}
