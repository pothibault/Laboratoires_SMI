#include "Includes/pwm.h"
#include "Includes/gpio.h"
#include "stdint.h"


void PWM_Init_PA5_TIM2(uint32_t timclk_hz, uint32_t frequency_hz, uint8_t duty_percent) {

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	GPIO_initPin(GPIOA, 0, GPIO_AF); //active pin en alternating function
    GPIOA->AFR[0] &= ~(0xFU << (5*4));   //met à 0 les 4 bits [23:20] pour GPIOx_AFRL
    GPIOA->AFR[0] |=  (0x1U << (5*4));   // ecrit la valeur 0x1 dans [23:20]
    GPIOA->OTYPER &= ~(1U<<5);//Actif haut et bas, fort courant, fronts rapides. Mets pour pin 5 mode push-pull
    GPIOA->OSPEEDR |=  (2U<<(5*2)); //High speed
    GPIOA->PUPDR   &= ~(3U<<(5*2)); //No pull-up, pull-down

    uint32_t g_psc = (timclk_hz/1000000U) - 1U;          //determine le prescaler a avoir pour obtenir une frequence de comptage de 1Mhz

    
    uint32_t g_arr = ( (timclk_hz/(g_psc+1U)) / frequency_hz ) - 1U; //fixe la periode
    uint32_t ccr = ((uint64_t)(g_arr+1U) * duty_percent)/100U; //fixe le nombre pour obtenir le duty cycle demander dans la periode de comptage
    if (ccr > g_arr) ccr = g_arr; //pour eviter depassement en cas de ARR de 100%

    TIM2->CR1   = 0;
    TIM2->CCMR1 = 0; //efface la config des sorties CH1/CH2
    TIM2->CCER  = 0;

    TIM2->CCMR1 |= (6U<<4) | TIM_CCMR1_OC1PE; // PWM mode 1 sur CH1 + CCR preload
    TIM2->CCER  |= TIM_CCER_CC1E; // active la sortie CH1 vers la pin PA5 en AF1
    TIM2->CR1   |= TIM_CR1_ARPE; //preload ARR

    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->PSC  = g_psc;
    TIM2->ARR  = g_arr;
    TIM2->CCR1 = ccr;
    TIM2->EGR  = TIM_EGR_UG; //force un Update Event immédiat
    TIM2->CR1 |= TIM_CR1_CEN;
}
