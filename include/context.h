#ifndef MARTYNE_CONTEXT_H
#define MARTYNE_CONTEXT_H

#include <stdint.h>

/*
 * Stack frame built by vectors.S.
 * x[0..30] occupy offsets 0..240, ELR is at 248, SPSR at 256.
 * The final eight bytes are alignment padding.
 */
typedef struct __attribute__((aligned(16))) {
    uint64_t x[31];
    uint64_t elr;
    uint64_t spsr;
} exception_frame_t;

_Static_assert(sizeof(exception_frame_t) == 272,
               "exception_frame_t must match vectors.S frame layout");

#endif
