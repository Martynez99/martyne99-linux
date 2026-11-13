# M4 — Preemptive scheduler design

## Execution model

The first scheduler is single-core and runs all tasks at EL1h in one kernel address space. It is intentionally a kernel-thread scheduler, not yet a user-process scheduler.

- One idle context is the boot-time `kernel_main` loop.
- Two static worker threads use separate 4 KiB stacks.
- The EL1 physical Generic Timer fires at 10 Hz.
- Each timer IRQ acknowledges the GIC interrupt, rearms the timer, saves the interrupted task's exception frame, and selects the next task in round-robin order.
- The exception return path restores the selected task's GPRs, `ELR_EL1`, and `SPSR_EL1`, then executes `ERET`.

## Context-frame ABI

`exception_frame_t` must match the assembly layout in `vectors.S`:

| Offset | Contents |
|---:|---|
| 0–240 | x0–x30 |
| 248 | ELR_EL1 |
| 256 | SPSR_EL1 |
| 264–271 | alignment padding |

The total frame size is 272 bytes and 16-byte aligned. A compile-time assertion protects the C/assembly contract.

## Current constraints

- No SMP or per-CPU run queues.
- No user mode, syscalls, process address spaces, or isolation.
- No blocking states, sleep queues, priorities, or synchronization primitives.
- FP/SIMD state is not saved; kernel compilation uses `-mgeneral-regs-only`.
- Timer period is fixed at 100 ms for this bring-up milestone.
- Worker stacks are static and 4 KiB each.

## Validation

The QEMU smoke test must observe:
- MMU activation.
- Timer ticks continuing.
- Both worker threads making progress.
- At least six task switches and a scheduler self-test success message.
