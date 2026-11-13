#ifndef MARTYNE_GIC_H
#define MARTYNE_GIC_H

#include <stdint.h>

void gic_init(void);
uint32_t gic_handle_irq(void);

#endif
