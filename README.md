# MARTYNE LINUX

MARTYNE LINUX is a freestanding AArch64 hobby operating-system kernel, initially targeting QEMU `virt`.

## Current implementation

### M1 — Kernel foundation
- AArch64 entry point with EL2-to-EL1 transition when entered at EL2
- Dedicated stack and BSS initialization
- PL011 serial console
- Linker layout and Clang/LLD build

### M2 — Exceptions and timer (in progress)
- AArch64 exception vector table installed through `VBAR_EL1`
- Synchronous exception diagnostics (ESR_EL1, FAR_EL1, ELR_EL1)
- GICv2 Group 1 setup for QEMU `virt`
- EL1 physical Generic Timer configured for a 10 Hz tick
- IRQ dispatch and interrupt acknowledgement
- Timer tick reporting on the serial console

M2 currently uses QEMU `virt` memory-map constants. Flattened Device Tree discovery and runtime validation remain outstanding. Treat this as experimental until the QEMU timer-interrupt smoke test passes on your machine.

## Build requirements

On Debian/Ubuntu:

```sh
sudo apt update
sudo apt install clang lld llvm make qemu-system-arm
```

## Build and run

```sh
make clean
make
make run
```

For a boot and timer interrupt smoke test:

```sh
make qemu-test
```

The test expects both `MARTYNE LINUX: M1 boot OK` and at least one `timer: tick` line in the serial output. QEMU is intentionally terminated after five seconds.

## Platform assumptions

- AArch64 / ARMv8-A, QEMU `virt`, Cortex-A57
- QEMU GICv2 (`-machine virt,gic-version=2`)
- Kernel linked at `0x40080000`
- PL011 UART at `0x09000000`
- GIC distributor at `0x08000000`, CPU interface at `0x08010000`
- EL1 execution after early setup
- MMU remains disabled; IRQs are enabled only after vector and GIC setup

## Layout

```text
include/{uart,exception,gic,timer}.h
src/arch/aarch64/{boot,vectors}.S
src/arch/aarch64/{uart,exception,gic,timer}.c
src/kernel.c
linker.ld
Makefile
docs/ROADMAP.md
```

See [docs/ROADMAP.md](docs/ROADMAP.md) for the staged subsystem plan.
