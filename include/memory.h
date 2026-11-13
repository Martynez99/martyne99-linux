#ifndef MARTYNE_MEMORY_H
#define MARTYNE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096UL

/* The initial QEMU virt memory window is supplied by the platform layer. */
int memory_init(uintptr_t ram_start, uintptr_t ram_end, uintptr_t reserved_end);
void *page_alloc(void);
int page_free(void *page);
size_t memory_free_pages(void);

int heap_init(void);
void *kmalloc(size_t size);
int kfree(void *ptr);

#endif
