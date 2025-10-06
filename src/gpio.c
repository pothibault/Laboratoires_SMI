#include "Includes/gpio.h"
#include "Includes/macros_utiles.h"
#include "stdint.h"

void GPIO_initPin(GPIO_TypeDef* port, uint8_t pin, GPIONode_t mode) {

	// pin vers bits
    uint8_t bit1 = pin*2;
    uint8_t bit2 = bit1+1;

    // Activation horloge
    if (port == GPIOA){
    	RCC->AHB1ENR |= BIT0;
    } else if (port == GPIOB){
       	RCC->AHB1ENR |= BIT1;
    } else if (port == GPIOC){
    	RCC->AHB1ENR |= BIT2;
    } else if (port == GPIOD){
    	RCC->AHB1ENR |= BIT3;
    } else if (port == GPIOE){
    	RCC->AHB1ENR |= BIT4;
    } else if (port == GPIOF){
    	RCC->AHB1ENR |= BIT5;
    } else if (port == GPIOG){
    	RCC->AHB1ENR |= BIT6;
    } else if (port == GPIOH){
    	RCC->AHB1ENR |= BIT7;
    }

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
	switch(bit){
	case 0:
		port->ODR &= ~convertPinToBit(pin);
		break;
	case 1:
		port->ODR |= convertPinToBit(pin);
		break;
	}
}

