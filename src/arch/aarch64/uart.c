#include "uart.h"

#include <stdint.h>

#define PL011_BASE 0x09000000UL
#define UARTDR     (*(volatile uint32_t *)(PL011_BASE + 0x000))
#define UARTFR     (*(volatile uint32_t *)(PL011_BASE + 0x018))
#define UARTIBRD   (*(volatile uint32_t *)(PL011_BASE + 0x024))
#define UARTFBRD   (*(volatile uint32_t *)(PL011_BASE + 0x028))
#define UARTLCR_H  (*(volatile uint32_t *)(PL011_BASE + 0x02c))
#define UARTCR     (*(volatile uint32_t *)(PL011_BASE + 0x030))
#define UARTIMSC   (*(volatile uint32_t *)(PL011_BASE + 0x038))
#define UARTICR    (*(volatile uint32_t *)(PL011_BASE + 0x044))

#define UARTFR_TXFF (1U << 5)

void uart_init(void)
{
    UARTCR = 0;
    UARTIMSC = 0;
    UARTICR = 0x7ff;
    // QEMU's PL011 clock is 24 MHz; 115200 baud => divisor 13 + 1/16.
    UARTIBRD = 13;
    UARTFBRD = 1;
    UARTLCR_H = (3U << 5) | (1U << 4); // 8 data bits, FIFO enabled
    UARTCR = (1U << 0) | (1U << 8) | (1U << 9); // UART, TX, RX
}

void uart_putc(char c)
{
    if (c == '\n')
        uart_putc('\r');

    while (UARTFR & UARTFR_TXFF)
        ;
    UARTDR = (uint32_t)c;
}

void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

void uart_hex(unsigned long value)
{
    static const char digits[] = "0123456789abcdef";
    uart_puts("0x");
    for (int shift = (int)(sizeof(value) * 8) - 4; shift >= 0; shift -= 4)
        uart_putc(digits[(value >> shift) & 0xf]);
}
