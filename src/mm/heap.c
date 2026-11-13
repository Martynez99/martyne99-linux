#include "memory.h"
#include <stdint.h>

#define HEAP_BYTES (64UL * 1024UL)
#define HEAP_MAGIC 0x4d415254U

typedef struct heap_block {
    size_t size;
    uint32_t magic;
    uint32_t free;
    struct heap_block *prev;
    struct heap_block *next;
} heap_block_t;

static uint8_t heap_area[HEAP_BYTES] __attribute__((aligned(16)));
static heap_block_t *heap_head;
static int heap_ready;

static size_t align_size(size_t size)
{
    return (size + 15UL) & ~15UL;
}

int heap_init(void)
{
    if (heap_ready)
        return -1;

    heap_head = (heap_block_t *)heap_area;
    heap_head->size = HEAP_BYTES - sizeof(*heap_head);
    heap_head->magic = HEAP_MAGIC;
    heap_head->free = 1;
    heap_head->prev = 0;
    heap_head->next = 0;
    heap_ready = 1;
    return 0;
}

void *kmalloc(size_t size)
{
    if (!heap_ready || size == 0 || size > HEAP_BYTES - sizeof(heap_block_t))
        return 0;

    size = align_size(size);
    for (heap_block_t *block = heap_head; block; block = block->next) {
        if (block->magic != HEAP_MAGIC)
            return 0;
        if (!block->free || block->size < size)
            continue;

        if (block->size >= size + sizeof(heap_block_t) + 16UL) {
            heap_block_t *split = (heap_block_t *)
                ((uint8_t *)(block + 1) + size);
            split->size = block->size - size - sizeof(*split);
            split->magic = HEAP_MAGIC;
            split->free = 1;
            split->prev = block;
            split->next = block->next;
            if (split->next)
                split->next->prev = split;
            block->next = split;
            block->size = size;
        }

        block->free = 0;
        return (void *)(block + 1);
    }
    return 0;
}

int kfree(void *ptr)
{
    if (!heap_ready || !ptr)
        return -1;

    heap_block_t *block = heap_head;
    while (block && (void *)(block + 1) != ptr)
        block = block->next;
    if (!block || block->magic != HEAP_MAGIC || block->free)
        return -1;

    block->free = 1;
    if (block->next && block->next->magic == HEAP_MAGIC && block->next->free) {
        heap_block_t *next = block->next;
        block->size += sizeof(*next) + next->size;
        block->next = next->next;
        if (block->next)
            block->next->prev = block;
    }
    if (block->prev && block->prev->magic == HEAP_MAGIC && block->prev->free) {
        heap_block_t *prev = block->prev;
        prev->size += sizeof(*block) + block->size;
        prev->next = block->next;
        if (prev->next)
            prev->next->prev = prev;
    }
    return 0;
}
