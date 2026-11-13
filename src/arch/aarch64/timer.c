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
     * EL1 physical timer. boot.S grants EL1 physical counter/timer access
     * through CNTHCTL_EL2. Its non-secure PPI is GIC INTID 30.
     */
    unsigned long control = 1;
    __asm__ volatile("msr cntp_ctl_el0, xzr" ::: "memory");
    __asm__ volatile("msr cntp_tval_el0, %0" :: "r"(interval) : "memory");
    __asm__ volatile("isb" ::: "memory");
    __asm__ volatile("msr cntp_ctl_el0, %0" :: "r"(control) : "memory");
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
