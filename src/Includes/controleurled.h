
#include "stm32f4xx.h"

typedef enum {
    OFF = 0,
    ON = 1
} LED_state;

void controleurled_init(GPIO_TypeDef* portButton, uint8_t pinButton,
		GPIO_TypeDef* portLED, uint8_t pinLED,ADC_TypeDef *adc);
void controleurled_turnOnOffLed(LED_state state, ADC_TypeDef *adc);
void controleurled_adjustIntensity(ADC_TypeDef *adc);
uint8_t controleurled_buttonPressed(GPIO_TypeDef* port, uint8_t pin);

