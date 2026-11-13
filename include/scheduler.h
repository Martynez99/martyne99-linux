#ifndef MARTYNE_SCHEDULER_H
#define MARTYNE_SCHEDULER_H

#include "context.h"

void scheduler_init(void);
exception_frame_t *scheduler_on_timer(exception_frame_t *frame);
unsigned long scheduler_switch_count(void);
int scheduler_self_test_passed(void);

#endif
