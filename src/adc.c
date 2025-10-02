#include "stm32f4xx.h"
#include "Includes/macros_utiles.h"
#include "stdint.h"

void ADC_init(ADC_TypeDef *adc){

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;   // Enable ADC1 clock
/*
	adc->CR2 |= BIT30; // SWSTART pour regular channel
	adc->CR1 |= BIT5;  // EOCIE enable pour interruption
	adc->SQR3 |= (BIT0 | BIT2 | BIT3); // Sequence de conversion, IN 13, une seule conversion
	NVIC->ISER[0] |= BIT18;	// Interruptions provenant de l'ADC, bit18 dans ISER 0
*/
	adc->CR2 = 0;                         // Ensure reset state
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
	    return (uint16_t)adc->DR;                // Lecture resultat 12 bit

}
