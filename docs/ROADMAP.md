# MARTYNE LINUX roadmap

The kernel is developed in dependency order. Each milestone should build and boot in QEMU before the next one is merged.

## M1 — Bootable kernel foundation
- [x] AArch64 entry point and EL2-to-EL1 transition
- [x] Stack and BSS initialization
- [x] PL011 serial output
- [x] Linker script and reproducible build targets
- [ ] Validate boot on the developer's QEMU installation

## M2 — CPU, exceptions, and interrupt controller
- [ ] Exception-level and CPU feature reporting
- [ ] AArch64 exception vectors and fault diagnostics
- [ ] Generic Timer driver
- [ ] Flattened Device Tree (FDT) discovery
- [ ] GICv2 interrupt-controller driver for QEMU virt
- [ ] Interrupt-safe logging and IRQ enable/disable primitives

## M3 — Memory manager
- [ ] Parse usable RAM from the device tree
- [ ] Physical page/frame allocator
- [ ] AArch64 page tables and virtual-memory map
- [ ] Kernel heap allocator
- [ ] Memory-region permissions and guard pages

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
