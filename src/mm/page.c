#include "memory.h"
#include <stdint.h>

#define MAX_RAM_BYTES (256UL * 1024UL * 1024UL)
#define MAX_FRAMES (MAX_RAM_BYTES / PAGE_SIZE)
#define BITMAP_BYTES (MAX_FRAMES / 8UL)

static uint8_t frame_bitmap[BITMAP_BYTES];
static uintptr_t ram_base;
static size_t frame_count;
static size_t first_free_frame;
static size_t free_count;
static int initialized;

static uintptr_t align_up(uintptr_t value, uintptr_t alignment)
{
    return (value + alignment - 1UL) & ~(alignment - 1UL);
}

static int frame_is_used(size_t index)
{
    return (frame_bitmap[index >> 3] >> (index & 7U)) & 1U;
}

static void frame_mark(size_t index, int used)
{
    uint8_t mask = (uint8_t)(1U << (index & 7U));
    if (used)
        frame_bitmap[index >> 3] |= mask;
    else
        frame_bitmap[index >> 3] &= (uint8_t)~mask;
}

int memory_init(uintptr_t start, uintptr_t end, uintptr_t reserved_end)
{
    if (initialized || start >= end || (start & (PAGE_SIZE - 1UL)) ||
        (end & (PAGE_SIZE - 1UL)) || end - start > MAX_RAM_BYTES ||
        reserved_end < start || reserved_end > end)
        return -1;

    uintptr_t usable_start = align_up(reserved_end, PAGE_SIZE);
    if (usable_start >= end)
        return -1;

    ram_base = start;
    frame_count = (size_t)((end - start) / PAGE_SIZE);
    for (size_t i = 0; i < BITMAP_BYTES; ++i)
        frame_bitmap[i] = 0xffU;

    first_free_frame = (size_t)((usable_start - start) / PAGE_SIZE);
    for (size_t i = first_free_frame; i < frame_count; ++i)
        frame_mark(i, 0);

    free_count = frame_count - first_free_frame;
    initialized = 1;
    return 0;
}

void *page_alloc(void)
{
    if (!initialized || free_count == 0)
        return 0;

    for (size_t i = first_free_frame; i < frame_count; ++i) {
        if (!frame_is_used(i)) {
            frame_mark(i, 1);
            --free_count;
            return (void *)(ram_base + i * PAGE_SIZE);
        }
    }
    return 0;
}

int page_free(void *page)
{
    uintptr_t address = (uintptr_t)page;
    if (!initialized || !page || (address & (PAGE_SIZE - 1UL)) ||
        address < ram_base || address >= ram_base + frame_count * PAGE_SIZE)
        return -1;

    size_t index = (size_t)((address - ram_base) / PAGE_SIZE);
    if (index < first_free_frame || !frame_is_used(index))
        return -1;

    frame_mark(index, 0);
    ++free_count;
    return 0;
}

size_t memory_free_pages(void)
{
    return initialized ? free_count : 0;
}
