# MARTYNE LINUX

MARTYNE LINUX is a freestanding AArch64 hobby operating-system kernel, initially targeting QEMU `virt`.

## M1 — Kernel foundation

- AArch64 entry point with EL2-to-EL1 transition when entered at EL2
- Dedicated stack and BSS initialization
- PL011 UART console
- Linker layout and Clang/LLD build
- QEMU run and boot-smoke-test targets

This is early kernel bring-up, not yet a general-purpose OS. Interrupts, virtual memory, scheduling, storage, networking, and graphics are future milestones.

## Build requirements

On Debian/Ubuntu:

```sh
sudo apt update
sudo apt install clang lld llvm make qemu-system-arm
```

## Build and run

```sh
make
make run
```

For a short serial boot check:

```sh
make qemu-test
```

The smoke test looks for `MARTYNE LINUX: M1 boot OK`. QEMU is expected to time out after the kernel reaches its idle loop; that timeout is normal.

## Layout

```text
include/uart.h
src/arch/aarch64/boot.S
src/arch/aarch64/uart.c
src/kernel.c
linker.ld
Makefile
docs/ROADMAP.md
```

## Initial platform assumptions

- AArch64 / ARMv8-A, QEMU `virt`, Cortex-A57
- Kernel linked at `0x40080000`
- PL011 UART at `0x09000000`
- EL1 execution after early setup
- MMU disabled and interrupts masked in M1

## Engineering direction

Architecture-specific code stays under `src/arch/aarch64`; drivers expose small interfaces; interrupts remain masked until vectors and the GIC are initialized; memory allocation is deferred until a physical allocator exists. See [the roadmap](docs/ROADMAP.md).
