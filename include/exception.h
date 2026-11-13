#ifndef MARTYNE_EXCEPTION_H
#define MARTYNE_EXCEPTION_H

#include "context.h"

void exception_vectors_install(void);
exception_frame_t *exception_dispatch(unsigned long vector,
                                      unsigned long esr,
                                      unsigned long far,
                                      unsigned long elr,
                                      exception_frame_t *frame);

#endif
