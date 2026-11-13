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
    __asm__ volatile("msr cntp_tval_el0, %0" :: "r"(interval));
    __asm__ volatile("msr cntp_ctl_el0, %0" :: "r"(1UL));
    __asm__ volatile("isb");
}

void timer_init(void)
{
    unsigned long frequency = read_counter_frequency();
    interval = frequency / 10UL; // 10 Hz periodic tick.
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
