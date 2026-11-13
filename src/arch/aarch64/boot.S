 .section .text.boot, "ax"
.global _start
.type _start, %function

_start:
    // QEMU may enter at EL2 or EL1. Drop to EL1h if necessary.
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    1f

    mov     x0, #(1 << 31)       // HCR_EL2.RW: EL1 is AArch64
    msr     hcr_el2, x0
    mov     x0, #3               // Permit EL1 physical counter/timer access
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    mov     x0, #0x3c5           // DAIF masked, EL1h
    msr     spsr_el2, x0
    adr     x0, 1f
    msr     elr_el2, x0
    isb
    eret

1:
    // Force EL1h so exceptions and all kernel threads use SP_EL1.
    msr     spsel, #1
    isb

    // Mask IRQ/FIQ/SError during early single-core bring-up.
    msr     daifset, #0xf

    // Install a private stack. QEMU virt RAM begins at 0x40000000.
    ldr     x0, =__stack_top
    mov     sp, x0

    // Clear the kernel BSS before entering C.
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
2:
    cmp     x0, x1
    b.hs    3f
    str     xzr, [x0], #8
    b       2b

3:
    bl      kernel_main

4:
    wfe
    b       4b

.size _start, . - _start

.section .bss.stack, "aw", %nobits
.align 12
.global __stack_bottom
__stack_bottom:
    .skip 16384
.global __stack_top
__stack_top:
