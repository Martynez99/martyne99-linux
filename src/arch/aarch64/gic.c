#include "gic.h"
#include "timer.h"
#include "uart.h"

#include <stdint.h>

#define GICD_BASE 0x08000000UL
#define GICC_BASE 0x08010000UL
#define GICD_CTLR (*(volatile uint32_t *)(GICD_BASE + 0x000))
#define GICD_IGROUPR0 (*(volatile uint32_t *)(GICD_BASE + 0x080))
#define GICD_ISENABLER0 (*(volatile uint32_t *)(GICD_BASE + 0x100))
#define GICD_IPRIORITYR (*(volatile uint8_t *)(GICD_BASE + 0x400 + 30))
#define GICC_CTLR (*(volatile uint32_t *)(GICC_BASE + 0x000))
#define GICC_PMR (*(volatile uint32_t *)(GICC_BASE + 0x004))
#define GICC_IAR (*(volatile uint32_t *)(GICC_BASE + 0x00c))
#define GICC_EOIR (*(volatile uint32_t *)(GICC_BASE + 0x010))

#define ARM_GENERIC_TIMER_PPI 30U
#define GIC_SPURIOUS_ID 1023U

static unsigned int unexpected_irq_reports;

void gic_init(void)
{
    /* QEMU virt GICv2: route the EL1 non-secure physical timer to Group 1. */
    GICD_CTLR = 0;
    GICC_CTLR = 0;

    GICD_IGROUPR0 |= (1U << ARM_GENERIC_TIMER_PPI);
    GICD_IPRIORITYR = 0xa0;
    GICD_ISENABLER0 = (1U << ARM_GENERIC_TIMER_PPI);

    GICC_PMR = 0xff;
    __asm__ volatile("dsb sy" ::: "memory");
    GICC_CTLR = 0x2;
    GICD_CTLR = 0x2;
    __asm__ volatile("dsb sy\n\tisb" ::: "memory");
}

void gic_handle_irq(void)
{
    uint32_t iar = GICC_IAR;
    uint32_t id = iar & 0x3ffU;

    if (id == GIC_SPURIOUS_ID)
        return;

    if (id == ARM_GENERIC_TIMER_PPI) {
        timer_irq_handler();
    } else if (unexpected_irq_reports < 4U) {
        ++unexpected_irq_reports;
        uart_puts("GIC: unexpected IRQ INTID ");
        uart_hex(id);
        uart_puts("\n");
    }

    GICC_EOIR = iar;
    __asm__ volatile("dsb sy" ::: "memory");
}
