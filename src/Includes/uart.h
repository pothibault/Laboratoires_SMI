#ifndef UART_H_
#define UART_H_

#include <stdint.h>

extern volatile int UART_DelayX;

void UART5_init(uint32_t apb1_clk_hz, uint32_t baud);
void UART5_putc(uint8_t c);
void UART5_sendString(const char *s);
int  UART5_getc_nonblocking(uint8_t *c);

#endif
