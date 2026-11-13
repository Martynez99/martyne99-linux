#include "scheduler.h"

#include <stdint.h>

#define THREAD_STACK_SIZE 4096UL
#define TASK_RUNNABLE 0U
#define TASK_RUNNING  1U

typedef struct kernel_task {
    exception_frame_t *frame;
    void (*entry)(void *);
    void *arg;
    volatile unsigned long progress;
    unsigned long id;
    unsigned int state;
} kernel_task_t;

static kernel_task_t idle_task;
static kernel_task_t workers[2];
static uint8_t worker_stacks[2][THREAD_STACK_SIZE]
    __attribute__((aligned(16)));

static kernel_task_t *current_task;
static volatile unsigned long switches;

static void worker_a(void *arg)
{
    kernel_task_t *task = (kernel_task_t *)arg;
    for (;;)
        ++task->progress;
}

static void worker_b(void *arg)
{
    kernel_task_t *task = (kernel_task_t *)arg;
    for (;;)
        ++task->progress;
}

/*
 * New tasks enter here through ERET with x0 pointing at their task record.
 * Kernel threads currently share EL1 and the kernel address space.
 */
static void thread_trampoline(kernel_task_t *task)
{
    task->entry(task->arg);
    for (;;)
        __asm__ volatile("wfe");
}

static void task_init(kernel_task_t *task, unsigned long id,
                      void (*entry)(void *), void *arg,
                      uint8_t *stack, unsigned long stack_size)
{
    task->id = id;
    task->entry = entry;
    task->arg = arg;
    task->progress = 0;
    task->state = TASK_RUNNABLE;

    uintptr_t stack_top = (uintptr_t)(stack + stack_size);
    exception_frame_t *frame =
        (exception_frame_t *)(stack_top - sizeof(exception_frame_t));

    uint64_t *words = (uint64_t *)frame;
    for (unsigned long i = 0; i < sizeof(*frame) / sizeof(uint64_t); ++i)
        words[i] = 0;

    frame->x[0] = (uintptr_t)task;
    frame->elr = (uintptr_t)thread_trampoline;

    /*
     * Return to EL1h. Keep debug, SError and FIQ masked; allow IRQs so
     * the periodic timer can preempt this thread.
     */
    frame->spsr = 0x345UL;
    task->frame = frame;
}

void scheduler_init(void)
{
    idle_task.frame = 0;
    idle_task.entry = 0;
    idle_task.arg = 0;
    idle_task.progress = 0;
    idle_task.id = 0;
    idle_task.state = TASK_RUNNING;

    task_init(&workers[0], 1, worker_a, &workers[0],
              worker_stacks[0], THREAD_STACK_SIZE);
    task_init(&workers[1], 2, worker_b, &workers[1],
              worker_stacks[1], THREAD_STACK_SIZE);

    current_task = &idle_task;
    switches = 0;
}

exception_frame_t *scheduler_on_timer(exception_frame_t *frame)
{
    if (!current_task || !frame)
        return frame;

    /* Save the interrupted task's complete EL1 exception context. */
    current_task->frame = frame;
    current_task->state = TASK_RUNNABLE;

    kernel_task_t *next;
    if (current_task == &idle_task)
        next = &workers[0];
    else if (current_task == &workers[0])
        next = &workers[1];
    else
        next = &idle_task;

    if (!next->frame) {
        current_task->state = TASK_RUNNING;
        return frame;
    }

    next->state = TASK_RUNNING;
    current_task = next;
    ++switches;
    return next->frame;
}

unsigned long scheduler_switch_count(void)
{
    return switches;
}

int scheduler_self_test_passed(void)
{
    return switches >= 6UL &&
           workers[0].progress != 0UL &&
           workers[1].progress != 0UL;
}
