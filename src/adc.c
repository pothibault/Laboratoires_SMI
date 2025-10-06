#include "stm32f4xx.h"
#include "Includes/macros_utiles.h"
#include "stdint.h"

static volatile uint16_t adc_value = 0;
static volatile uint8_t adc_ready = 0;

void ADC_init(ADC_TypeDef *adc){

	// Start le clock
	if(adc == ADC1){
		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	} else if (adc == ADC2){
		RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
	} else if (adc == ADC3){
		RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
	}

	adc->CR1 |= ADC_CR1_EOCIE;  // EOCIE enable pour interruption
	NVIC->ISER[0] |= (1 << 18); // ADC_IRQn = 18
	adc->CR2 = 0;                         // Etat reset
	adc->CR2 |= ADC_CR2_ADON;             // On active l'ADC, regular channel

	adc->SQR1 = 0;                        // Une conversion seulement
	adc->SQR3 = 13;                       // Channel 13 = PC3
	adc->SMPR1 |= ADC_SMPR1_SMP13_1;      // Sample time, ici 55.5 cycles

}

void ADC_startConversion(ADC_TypeDef *adc){
	adc->CR2 |= ADC_CR2_SWSTART;
}

uint16_t ADC_getSample(ADC_TypeDef *adc){
		ADC_startConversion(adc);
	    while (!(adc->SR & ADC_SR_EOC));         // Attente fin de la conversion
	    return (uint16_t)adc->DR;                // Lecture resultat 12 bit (maximum du stm)
}

void ADC_IRQHandler(ADC_TypeDef *adc){
    if (ADC1->SR & ADC_SR_EOC) {
        adc_value = ADC1->DR;
        adc_ready = 1;
    }
}

uint8_t ADC_isReady(void) {
	return adc_ready;
}
uint16_t ADC_readValue(void) {
	adc_ready = 0;
	return adc_value;
}

