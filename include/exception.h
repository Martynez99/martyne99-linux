#ifndef MARTYNE_EXCEPTION_H
#define MARTYNE_EXCEPTION_H

void exception_vectors_install(void);
void exception_dispatch(unsigned long vector, unsigned long esr,
                        unsigned long far, unsigned long elr);

#endif
