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
    /* EL1 virtual timer: its PPI is INTID 27 on the QEMU virt GIC. */
    __asm__ volatile("msr cntv_tval_el0, %0" :: "r"(interval) : "memory");
    __asm__ volatile("msr cntv_ctl_el0, %0" :: "r"(1UL) : "memory");
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
