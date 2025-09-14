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
    } else if (port == GPIOG){
    	RCC->AHB1ENR |= BIT6;
    }

    if (mode == GPIO_OUTPUT){
        //Reset
        port->MODER &= ~(convertPinToBit(bit1) | convertPinToBit(bit2));
        //Set
        port->MODER |=  convertPinToBit(bit1);
    } else if (mode == GPIO_INPUT){
    	port->MODER &= ~(convertPinToBit(bit1) | convertPinToBit(bit2));
    }


}

int GPIO_readPin(GPIO_TypeDef* port, uint8_t pin){
	//uint32_t pinBit = convertPinToBit(pin);
	return (port->IDR & convertPinToBit(pin));
}

void GPIO_writePin(GPIO_TypeDef* port, uint8_t pin, uint8_t bit) {
	//uint32_t pinBit = convertPinToBit(pin);
	switch(bit){
	case 0:
		port->ODR &= ~convertPinToBit(pin);
		break;
	case 1:
		port->ODR |= convertPinToBit(pin);
		break;
	}
}

