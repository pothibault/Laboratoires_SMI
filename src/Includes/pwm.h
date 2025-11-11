#ifndef PWM_H
#define PWM_H

#include "stdint.h"
#include "stm32f4xx.h"

typedef struct {
    TIM_TypeDef *tim;        // ex: TIM2, TIM3, ...
    uint8_t      channel;    // 1..4
    uint32_t     timclk_hz;  // horloge d'entrée du timer
} PWM_Handle_t;


void PWM_InitTimer(PWM_Handle_t *hpwm, TIM_TypeDef *tim, uint8_t channel, uint32_t timclk_hz);

uint32_t PWM_SetFrequency(PWM_Handle_t *hpwm, uint32_t frequency_hz);

void PWM_SetDuty(PWM_Handle_t *hpwm, uint8_t duty_percent);

void PWM_Start(PWM_Handle_t *hpwm);
void PWM_Stop(PWM_Handle_t *hpwm);

#endif
