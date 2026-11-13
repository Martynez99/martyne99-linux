#include "gic.h"
#include "timer.h"

#include <stdint.h>

#define GICD_BASE       0x08000000UL
#define GICC_BASE       0x08010000UL
#define GICD_CTLR       (*(volatile uint32_t *)(GICD_BASE + 0x000))
#define GICD_IGROUPR0   (*(volatile uint32_t *)(GICD_BASE + 0x080))
#define GICD_ISENABLER0 (*(volatile uint32_t *)(GICD_BASE + 0x100))
#define GICD_IPRIORITYR (*(volatile uint8_t  *)(GICD_BASE + 0x400 + 30))
#define GICC_CTLR       (*(volatile uint32_t *)(GICC_BASE + 0x000))
#define GICC_PMR        (*(volatile uint32_t *)(GICC_BASE + 0x004))
#define GICC_IAR        (*(volatile uint32_t *)(GICC_BASE + 0x00c))
#define GICC_EOIR       (*(volatile uint32_t *)(GICC_BASE + 0x010))

#define ARM_GENERIC_TIMER_PPI 30U
#define GIC_SPURIOUS_ID        1023U

void gic_init(void)
{
    // QEMU virt's GICv2: route the non-secure physical timer PPI to Group 1.
    GICD_CTLR = 0;
    GICC_CTLR = 0;

    GICD_IGROUPR0 |= (1U << ARM_GENERIC_TIMER_PPI);
    GICD_IPRIORITYR = 0xa0;
    GICD_ISENABLER0 = (1U << ARM_GENERIC_TIMER_PPI);

    GICC_PMR = 0xff;
    GICC_CTLR = 0x2; // Enable Group 1 at the CPU interface.
    GICD_CTLR = 0x2; // Enable Group 1 at the distributor.
}

void gic_handle_irq(void)
{
    uint32_t iar = GICC_IAR;
    uint32_t id = iar & 0x3ffU;

    if (id == GIC_SPURIOUS_ID)
        return;

    if (id == ARM_GENERIC_TIMER_PPI)
        timer_irq_handler();

    // Write back the full acknowledge value, including the CPU source ID.
    GICC_EOIR = iar;
}
