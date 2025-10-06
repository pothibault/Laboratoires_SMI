#ifndef ADC_H
#define ADC_H

#include "stm32f4xx.h"

void ADC_init(ADC_TypeDef *adc);
void ADC_startConversion(ADC_TypeDef *adc);
uint16_t ADC_getSample(ADC_TypeDef *adc);
void ADC_IRQHandler(ADC_TypeDef *adc);
uint8_t ADC_isReady(void);
uint16_t ADC_readValue(void);
void ADC_setIsReady(uint8_t value);
#endif
