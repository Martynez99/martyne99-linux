.section .text.vectors, "ax"
.balign 2048
.global exception_vectors
.type exception_vectors, %function

.macro VECTOR target
    b \target
    .space 124
.endm

exception_vectors:
    // Current EL using SP0
    VECTOR sync_exception
    VECTOR irq_exception
    VECTOR sync_exception
    VECTOR sync_exception
    // Current EL using SPx
    VECTOR sync_exception
    VECTOR irq_exception
    VECTOR sync_exception
    VECTOR sync_exception
    // Lower EL using AArch64
    VECTOR sync_exception
    VECTOR irq_exception
    VECTOR sync_exception
    VECTOR sync_exception
    // Lower EL using AArch32
    VECTOR sync_exception
    VECTOR irq_exception
    VECTOR sync_exception
    VECTOR sync_exception

.macro SAVE_CONTEXT
    sub sp, sp, #256
    stp x0,  x1,  [sp, #0]
    stp x2,  x3,  [sp, #16]
    stp x4,  x5,  [sp, #32]
    stp x6,  x7,  [sp, #48]
    stp x8,  x9,  [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
.endm

.macro RESTORE_CONTEXT
    ldp x0,  x1,  [sp, #0]
    ldp x2,  x3,  [sp, #16]
    ldp x4,  x5,  [sp, #32]
    ldp x6,  x7,  [sp, #48]
    ldp x8,  x9,  [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]
    add sp, sp, #256
.endm

sync_exception:
    SAVE_CONTEXT
    mov x0, #0
    b exception_common

irq_exception:
    SAVE_CONTEXT
    mov x0, #1

exception_common:
    // C ABI: x0=vector kind, x1=ESR, x2=FAR, x3=ELR.
    mrs x1, esr_el1
    mrs x2, far_el1
    mrs x3, elr_el1
    bl exception_dispatch

    RESTORE_CONTEXT
    eret

.size exception_vectors, . - exception_vectors
