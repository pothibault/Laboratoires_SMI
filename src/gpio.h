#include "stm32f4xx.h" // Replace with your specific device header

// Enum for pin modes
typedef enum {
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1,
    GPIO_AF = 2,
    GPIO_ANALOG = 3
} GPIONode_t;


void GPIO_initPin(GPIO_TypeDef* port, uint8_t pin, GPIONode_t mode);

void GPIO_writePin(GPIO_TypeDef* port, uint8_t pin, uint8_t bit);

int GPIO_readPin(GPIO_TypeDef* port, uint8_t pin);
