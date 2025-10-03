#ifndef PWM_H
#define PWM_H

#include "stdint.h"


void PWM_Init_PA5_TIM2(uint32_t timclk_hz, uint32_t frequency_hz, uint8_t duty_percent);

void PWM_SetDuty(int duty_cycle);

void PWM_SetFrequency(int frequency);

#endif
