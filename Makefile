TARGET      := aarch64-none-elf
BUILD       := build
KERNEL      := $(BUILD)/martyne.elf

CC          := clang
LD          := ld.lld

CFLAGS      := --target=$(TARGET) -mcpu=cortex-a57 -ffreestanding -fno-builtin \
               -fno-stack-protector -fno-pic -fno-pie -mgeneral-regs-only \
               -Wall -Wextra -Werror -O2 -g -Iinclude
ASFLAGS     := --target=$(TARGET) -mcpu=cortex-a57 -g
LDFLAGS     := -T linker.ld -nostdlib --build-id=none -Map=$(BUILD)/martyne.map

C_SOURCES   := src/kernel.c src/arch/aarch64/uart.c \
               src/arch/aarch64/exception.c src/arch/aarch64/gic.c \
               src/arch/aarch64/timer.c
ASM_SOURCES := src/arch/aarch64/boot.S src/arch/aarch64/vectors.S
OBJECTS     := $(patsubst %.c,$(BUILD)/%.o,$(C_SOURCES)) \
               $(patsubst %.S,$(BUILD)/%.o,$(ASM_SOURCES))

.PHONY: all clean run qemu-test
all: $(KERNEL)

$(KERNEL): $(OBJECTS) linker.ld
	@mkdir -p $(BUILD)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	@echo "Built $@"

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

run: $(KERNEL)
	qemu-system-aarch64 -machine virt,gic-version=2 -cpu cortex-a57 -m 256M \
		-nographic -monitor none -serial stdio -kernel $(KERNEL)

qemu-test: $(KERNEL)
	@command -v timeout >/dev/null || (echo "timeout is required"; exit 1)
	@set +e; timeout 5s qemu-system-aarch64 -machine virt,gic-version=2 -cpu cortex-a57 -m 256M \
		-nographic -monitor none -serial stdio -kernel $(KERNEL) > $(BUILD)/qemu-test.log 2>&1; \
		status=$$?; set -e; cat $(BUILD)/qemu-test.log; \
		grep -q "MARTYNE LINUX: M1 boot OK" $(BUILD)/qemu-test.log || \
		(echo "Kernel boot signature missing (QEMU exit $$status)"; exit 1); \
		ticks=$(grep -c "timer: tick" $(BUILD)/qemu-test.log || true); \
		test "$ticks" -ge 3 || \
		(echo "Expected at least 3 timer ticks, observed $ticks (QEMU exit $status)"; exit 1); \
		echo "QEMU boot and repeated timer IRQ test passed"

clean:
	rm -rf $(BUILD)
