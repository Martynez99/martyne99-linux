#include "mmu.h"
#include <stdint.h>

#define PAGE_SIZE       4096UL
#define BLOCK_SIZE      (2UL * 1024UL * 1024UL)
#define RAM_BASE        0x40000000UL
#define RAM_SIZE        (256UL * 1024UL * 1024UL)
#define DEVICE_BASE_GIC 0x08000000UL
#define DEVICE_BASE_UART 0x09000000UL

#define DESC_VALID      (1UL << 0)
#define DESC_TABLE      (1UL << 1)
#define DESC_BLOCK      DESC_VALID
#define DESC_PAGE       (DESC_VALID | DESC_TABLE)
#define DESC_AF         (1UL << 10)
#define DESC_SH_INNER   (3UL << 8)
#define DESC_ATTR(n)    ((uint64_t)(n) << 2)
#define DESC_AP_EL1_RO  (2UL << 6)
#define DESC_PXN        (1UL << 53)
#define DESC_UXN        (1UL << 54)

#define ATTR_DEVICE     0U
#define ATTR_NORMAL     1U

static uint64_t l1_table[512] __attribute__((aligned(PAGE_SIZE)));
static uint64_t l2_low[512] __attribute__((aligned(PAGE_SIZE)));
static uint64_t l2_ram[512] __attribute__((aligned(PAGE_SIZE)));
static uint64_t l3_ram_first[512] __attribute__((aligned(PAGE_SIZE)));

extern char __text_start[];
extern char __text_end[];
extern char __rodata_start[];
extern char __rodata_end[];

static int enabled;

static uint64_t table_descriptor(const void *table)
{
    return ((uint64_t)(uintptr_t)table & 0x0000fffffffff000UL) |
           DESC_VALID | DESC_TABLE;
}

static uint64_t block_descriptor(uintptr_t address, unsigned int attr,
                                 int executable)
{
    uint64_t descriptor = (uint64_t)address | DESC_BLOCK | DESC_AF |
                          DESC_SH_INNER | DESC_ATTR(attr) | DESC_UXN;
    if (!executable)
        descriptor |= DESC_PXN;
    return descriptor;
}

static uint64_t page_descriptor(uintptr_t address, int type)
{
    uint64_t descriptor = (uint64_t)address | DESC_PAGE | DESC_AF |
                          DESC_SH_INNER | DESC_ATTR(ATTR_NORMAL) | DESC_UXN;

    if (type == 1) {
        /* Kernel text: EL1 read-only, executable. */
        descriptor |= DESC_AP_EL1_RO;
    } else {
        /* Data, BSS, and unused RAM: privileged RW, execute-never. */
        descriptor |= DESC_PXN;
        if (type == 2)
            descriptor |= DESC_AP_EL1_RO;
    }
    return descriptor;
}

static void build_tables(void)
{
    for (unsigned int i = 0; i < 512; ++i) {
        l1_table[i] = 0;
        l2_low[i] = 0;
        l2_ram[i] = 0;
        l3_ram_first[i] = 0;
    }

    /* Identity-map only the QEMU virt GIC and PL011 2 MiB MMIO windows. */
    l1_table[0] = table_descriptor(l2_low);
    l2_low[DEVICE_BASE_GIC / BLOCK_SIZE] =
        block_descriptor(DEVICE_BASE_GIC, ATTR_DEVICE, 0);
    l2_low[DEVICE_BASE_UART / BLOCK_SIZE] =
        block_descriptor(DEVICE_BASE_UART, ATTR_DEVICE, 0);

    /* Identity-map the configured 256 MiB RAM window. */
    l1_table[1] = table_descriptor(l2_ram);
    l2_ram[0] = table_descriptor(l3_ram_first);

    uintptr_t text_start = (uintptr_t)__text_start;
    uintptr_t text_end = (uintptr_t)__text_end;
    uintptr_t ro_start = (uintptr_t)__rodata_start;
    uintptr_t ro_end = (uintptr_t)__rodata_end;

    for (unsigned int i = 0; i < 512; ++i) {
        uintptr_t address = RAM_BASE + (uintptr_t)i * PAGE_SIZE;
        int type = 0;
        if (address >= text_start && address < text_end)
            type = 1;
        else if (address >= ro_start && address < ro_end)
            type = 2;
        l3_ram_first[i] = page_descriptor(address, type);
    }

    for (unsigned int i = 1; i < (RAM_SIZE / BLOCK_SIZE); ++i) {
        uintptr_t address = RAM_BASE + (uintptr_t)i * BLOCK_SIZE;
        l2_ram[i] = block_descriptor(address, ATTR_NORMAL, 0);
    }
}

int mmu_init(void)
{
    if (enabled)
        return -1;

    build_tables();

    uint64_t mair = 0x00UL | (0xffUL << 8); /* Device-nGnRnE, Normal WB */
    uint64_t tcr = 25UL | (25UL << 16) | (1UL << 8) | (1UL << 10) |
                   (3UL << 12) | (1UL << 23) | (2UL << 32);

    __asm__ volatile(
        "dsb sy\n"
        "msr mair_el1, %0\n"
        "msr tcr_el1, %1\n"
        "msr ttbr0_el1, %2\n"
        "isb\n"
        "tlbi vmalle1\n"
        "dsb sy\n"
        "isb\n"
        :
        : "r"(mair), "r"(tcr), "r"((uint64_t)(uintptr_t)l1_table)
        : "memory");

    uint64_t sctlr;
    __asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
    sctlr |= (1UL << 0) | (1UL << 2) | (1UL << 12); /* M, C, I */
    __asm__ volatile(
        "ic iallu\n"
        "dsb sy\n"
        "msr sctlr_el1, %0\n"
        "isb\n"
        :
        : "r"(sctlr)
        : "memory");

    enabled = 1;
    return 0;
}

int mmu_is_enabled(void)
{
    uint64_t sctlr;
    __asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
    return (sctlr & 1UL) != 0;
}
