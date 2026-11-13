# Memory management (M3)

M3 is being delivered in small, testable layers.

## M3a: physical frames and kernel heap

- QEMU virt RAM is initially configured as 256 MiB at physical address 0x40000000.
- The linker exports the kernel image bounds. The platform reserves all memory through the aligned kernel end.
- A fixed-capacity bitmap tracks 4 KiB physical frames. The bitmap itself lives in BSS and is therefore covered by the kernel reservation.
- `page_alloc()` returns one zero-state-independent physical frame; callers must initialize its contents.
- `page_free()` rejects unaligned, out-of-range, and already-free frames.
- A 64 KiB, 16-byte-aligned kernel heap provides first-fit allocation, splitting, and adjacent-block coalescing.

The frame allocator currently assumes the QEMU memory layout configured by the build. It is deliberately bounded to 256 MiB and rejects larger regions rather than overrunning its bitmap.

## M3b: virtual memory (next layer)

AArch64 translation tables, MAIR/TCR setup, identity mappings for RAM and MMIO, page permissions, and guard pages are not enabled by M3a. They require a reviewed memory map and a dedicated QEMU test before turning on the MMU.
