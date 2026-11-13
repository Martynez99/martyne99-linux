# M3b — AArch64 virtual memory

## Initial translation regime

- 4 KiB translation granule, 39-bit TTBR0_EL1 virtual address space.
- 3-level translation: L1 -> L2 -> L3 for the first 2 MiB of RAM.
- Identity mapping keeps the current PC, stack, kernel pointers, and physical addresses valid while enabling the MMU.
- QEMU virt GICv2 (0x08000000) and PL011 UART (0x09000000) are mapped as Device-nGnRnE.
- QEMU RAM at 0x40000000, configured as 256 MiB, is mapped as Normal cacheable memory.
- Kernel text pages are EL1 read-only and executable; rodata pages are EL1 read-only and execute-never; data/BSS pages are privileged RW and execute-never.
- RAM outside the first 2 MiB is mapped with 2 MiB blocks, privileged RW and execute-never.

## Deliberate limitations

This is a QEMU virt bring-up map, not yet a portable platform memory manager. It assumes the fixed RAM size and peripheral addresses passed by the QEMU command line. It does not parse the FDT, dynamically map devices, provide user mappings, or implement page-table editing/TLB shootdowns. The 2 MiB block mappings beyond the first 2 MiB do not yet enforce per-section permissions. Guard pages and demand paging are future work.

MMU activation must be tested on the actual QEMU setup before this milestone is considered complete. If the kernel stops printing immediately after the enable message, capture QEMU exception diagnostics and inspect the translation regime before changing the map.
