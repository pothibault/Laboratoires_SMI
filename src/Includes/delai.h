

#ifndef DELAI_H
#define DELAI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t timer_ms;
} timer_t;


void InitSysTick_1ms(uint32_t clk_hz);
uint32_t millis(void);
void timer_start(timer_t *t);
bool timer_expired(timer_t *t, uint32_t delay_ms);
void delay_ms_blocking(uint32_t ms);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* DELAI_H */
