#ifndef PWM_H
#define PWM_H

#include "stdint.h"


void PWM_Init(uint32_t timclk_hz, uint32_t frequency_hz, uint8_t duty_percent);

void PWM_SetFrequency(uint32_t timclk_hz, uint32_t freq_hz);

void PWM_SetDuty(uint8_t duty_percent);

#endif
