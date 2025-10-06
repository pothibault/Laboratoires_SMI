
#include "stm32f4xx.h"
#include "Includes/macros_utiles.h"
#include "Includes/gpio.h"
#include "Includes/adc.h"
#include "Includes/pwm.h"
#include "stdint.h"
#include "Includes/controleurled.h"

void controleurled_init(GPIO_TypeDef* portButton, uint8_t pinButton,
		GPIO_TypeDef* portLED, uint8_t pinLED,ADC_TypeDef *adc){
	GPIO_initPin(portButton,pinButton,GPIO_INPUT);
	GPIO_initPin(portLED,pinLED,GPIO_ANALOG);
	ADC_init(adc);
	SystemCoreClockUpdate();
	uint32_t clock = SystemCoreClock/2;
	PWM_Init_PA5_TIM2(clock, 100, 25);
}
void controleurled_turnOnOffLed(LED_state state, ADC_TypeDef *adc){
	switch(state){
	case ON:
		//Allume led selon intensite
		controleurled_adjustIntensity(adc);
	case OFF:
		//Ferme led
		PWM_SetDuty(0);
	}

}
void controleurled_adjustIntensity(ADC_TypeDef *adc){
	uint16_t sample = ADC_getSample(adc);
	uint8_t intensity;
	if (sample <= 0){
		PWM_SetDuty(0);
		return;
	}
	intensity = (sample/4095)*100;
	PWM_SetDuty(intensity);
	return;
}
uint8_t controleurled_buttonPressed(GPIO_TypeDef* port, uint8_t pin){
	return GPIO_readPin(port,pin);
}
