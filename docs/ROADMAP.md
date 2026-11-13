# MARTYNE LINUX roadmap

The kernel is developed in dependency order. Each milestone should build and boot in QEMU before the next one is merged.

## M1 — Bootable kernel foundation
- [x] AArch64 entry point and EL2-to-EL1 transition
- [x] Stack and BSS initialization
- [x] PL011 serial output
- [x] Linker script and reproducible build targets
- [ ] Validate boot on the developer's QEMU installation

## M2 — CPU, exceptions, and interrupt controller
- [x] AArch64 exception vector table and VBAR_EL1 installation
- [x] Synchronous exception diagnostics (ESR, FAR, ELR)
- [x] EL1 physical Generic Timer driver (10 Hz)
- [x] GICv2 Group 1 setup for QEMU virt
- [x] IRQ dispatch and timer interrupt acknowledgement
- [ ] Flattened Device Tree (FDT) discovery; current addresses are QEMU virt constants
- [ ] Runtime validation on the developer's QEMU installation

## M3 — Memory manager
- [x] Bounded 4 KiB physical frame allocator for the initial QEMU RAM window
- [x] Kernel heap with splitting and coalescing
- [ ] Parse usable RAM and reserved regions from the device tree
- [ ] AArch64 page tables and virtual-memory map
- [ ] Memory-region permissions and guard pages
- [ ] QEMU allocation, exhaustion, free, and reuse tests

## M4 — Processes and scheduling
- [ ] CPU context structure and context save/restore
- [ ] Kernel threads
- [ ] Round-robin preemptive scheduler
- [ ] Synchronization primitives
- [ ] User/kernel privilege boundary and syscall ABI

## M5 — Storage and filesystem
- [ ] VirtIO-MMIO block driver
- [ ] Block-device abstraction
- [ ] Initial RAM filesystem for early userland
- [ ] Persistent filesystem and file operations
- [ ] VFS mount and path interfaces

## M6 — Networking
- [ ] VirtIO network driver
- [ ] Ethernet frame layer
- [ ] ARP and IPv4
- [ ] UDP, then TCP
- [ ] Socket API and network diagnostics

## M7 — Graphics and input
- [ ] Framebuffer discovery through device tree
- [ ] Pixel and rectangle drawing primitives
- [ ] Basic console/window compositor
- [ ] Keyboard/input abstraction
- [ ] Optional VirtIO GPU support

## Engineering rules
- Keep hardware-specific code behind driver interfaces.
- Avoid heap use until the allocator is implemented.
- Keep interrupts masked until vectors and the GIC are initialized.
- Validate address ranges and device-tree data before using them.
- Add tests or a QEMU smoke test for each driver and subsystem.
