#include "timer.h"

static volatile unsigned long ticks;
static unsigned long interval;

static inline unsigned long read_counter_frequency(void)
{
    unsigned long value;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(value));
    return value;
}

static inline void arm_timer(void)
{
    /*
     * Use the EL1 physical timer. boot.S sets CNTHCTL_EL2 bits 0 and 1,
     * granting EL1 access to the physical counter and timer.
     * The non-secure physical timer is GIC PPI 30.
     */
    __asm__ volatile("msr cntp_ctl_el0, xzr" ::: "memory");
    __asm__ volatile("msr cntp_tval_el0, %0" :: "r"(interval) : "memory");
    __asm__ volatile("isb" ::: "memory");
    __asm__ volatile("mov x0, #1\n\tmsr cntp_ctl_el0, x0" ::: "x0", "memory");
    __asm__ volatile("isb" ::: "memory");
}

void timer_init(void)
{
    unsigned long frequency = read_counter_frequency();
    interval = frequency / 10UL;
    if (interval == 0)
        interval = 1;

    ticks = 0;
    arm_timer();
}

void timer_irq_handler(void)
{
    ++ticks;
    arm_timer();
}

unsigned long timer_ticks(void)
{
    return ticks;
}
