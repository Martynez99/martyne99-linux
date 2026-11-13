#ifndef MARTYNE_TIMER_H
#define MARTYNE_TIMER_H

void timer_init(void);
void timer_irq_handler(void);
unsigned long timer_ticks(void);

#endif
