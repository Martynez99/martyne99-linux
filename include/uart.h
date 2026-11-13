#ifndef MARTYNE_UART_H
#define MARTYNE_UART_H

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_hex(unsigned long value);

#endif
