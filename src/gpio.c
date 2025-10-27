#include "Includes/gpio.h"
#include "Includes/macros_utiles.h"
#include "stdint.h"

static inline uint32_t bitmask(uint8_t pin){ return (uint32_t)1u << pin; }


void GPIO_initPin(GPIO_TypeDef* port, uint8_t pin, GPIONode_t mode) {

	// pin vers bits
    uint8_t bit1 = pin*2;
    uint8_t bit2 = bit1+1;

    // Activation horloge du port
    if      (port == GPIOA) RCC->AHB1ENR |= BIT0;
    else if (port == GPIOB) RCC->AHB1ENR |= BIT1;
    else if (port == GPIOC) RCC->AHB1ENR |= BIT2;
    else if (port == GPIOD) RCC->AHB1ENR |= BIT3;
    else if (port == GPIOE) RCC->AHB1ENR |= BIT4;
    else if (port == GPIOF) RCC->AHB1ENR |= BIT5;
    else if (port == GPIOG) RCC->AHB1ENR |= BIT6;
    else if (port == GPIOH) RCC->AHB1ENR |= BIT7;

    //Reset
    port->MODER &= ~(convertPinToBit(bit1) | convertPinToBit(bit2));
    // Set
    if (mode == GPIO_OUTPUT){ //0:1
        port->MODER |= convertPinToBit(bit1);
    } else if (mode == GPIO_ANALOG){ //1:1
        port->PUPDR &= ~(3U << (pin*2));
    	port->MODER |= (convertPinToBit(bit1) | convertPinToBit(bit2));
    } else if (mode == GPIO_AF){ //1:0
    	port->MODER |= convertPinToBit(bit2);
    } else if (mode == GPIO_INPUT) {
        port->PUPDR &= ~(3U << (pin*2));
        port->PUPDR |=  (2U << (pin*2));
    }

}

int GPIO_readPin(GPIO_TypeDef* port, uint8_t pin){
	return ( (port->IDR & convertPinToBit(pin)) ? 1 : 0 );
}

void GPIO_writePin(GPIO_TypeDef* port, uint8_t pin, uint8_t bit) {
    if (bit) port->ODR |= convertPinToBit(pin);
    else port->ODR &= ~convertPinToBit(pin);
}

void GPIO_setAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num) {
    volatile uint32_t *AFR = (pin < 8) ? &port->AFR[0] : &port->AFR[1];
    uint8_t idx = (pin < 8) ? pin : (pin - 8);
    *AFR &= ~(0xFU << (idx * 4));
    *AFR |=  ((uint32_t)(af_num & 0xF) << (idx * 4));
}

void GPIO_configPWMPad(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num) {
    GPIO_initPin(port, pin, GPIO_AF);   // sets MODER = AF

    GPIO_setAF(port, pin, af_num);      // selects AFx

    port->OTYPER  &= ~(1u << pin);     // push-pull
    port->OSPEEDR |=  (2U << (pin * 2));// high speed
    port->PUPDR   &= ~(3U << (pin * 2));// no pull
}

void GPIO_configAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num,
                   GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd)
{
    
    GPIO_initPin(port, pin, GPIO_AF);

    
    GPIO_setAF(port, pin, af_num);

    
    if (otype == GPIO_OT_PP) port->OTYPER &= ~bitmask(pin);
    else                     port->OTYPER |=  bitmask(pin);

    port->OSPEEDR &= ~(3u << (pin*2));
    port->OSPEEDR |=  ((uint32_t)speed & 3u) << (pin*2);

    port->PUPDR &= ~(3u << (pin*2));
    port->PUPDR |=  ((uint32_t)pupd & 3u) << (pin*2);
}

void GPIO_configOutput(GPIO_TypeDef* port, uint8_t pin,
                       GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd)
{
    
    GPIO_initPin(port, pin, GPIO_OUTPUT);

    
    if (otype == GPIO_OT_PP) port->OTYPER &= ~bitmask(pin);
    else                     port->OTYPER |=  bitmask(pin);

    port->OSPEEDR &= ~(3u << (pin*2));
    port->OSPEEDR |=  ((uint32_t)speed & 3u) << (pin*2);

    port->PUPDR &= ~(3u << (pin*2));
    port->PUPDR |=  ((uint32_t)pupd & 3u) << (pin*2);
}
