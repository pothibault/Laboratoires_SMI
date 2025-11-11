
// #include "stm32f4xx.h"
// #include "Includes/macros_utiles.h"
// #include "Includes/gpio.h"
// #include "Includes/adc.h"
// #include "Includes/pwm.h"
// #include "stdint.h"
// #include "Includes/controleurled.h"

// void controleurled_init(GPIO_TypeDef* portButton, uint8_t pinButton,
// 		GPIO_TypeDef* portLED, uint8_t pinLED,ADC_TypeDef *adc){
// 	GPIO_initPin(portButton,pinButton,GPIO_INPUT);
// 	GPIO_initPin(portLED,pinLED,GPIO_ANALOG);
// 	ADC_init(adc);
// 	SystemCoreClockUpdate();
// 	uint32_t clock = SystemCoreClock/2;
// 	PWM_Init(clock, 100, 0);
// }

// void controleurled_turnOnOffLed(LED_state state, ADC_TypeDef *adc){
// 	switch(state){
// 	case ON:
// 		//Allume led selon intensite
// 		controleurled_adjustIntensity(adc);
// 		break;
// 	case OFF:
// 		//Ferme led
// 		PWM_SetDuty(0);
// 		break;
// 	}

// }

// void controleurled_adjustIntensity(ADC_TypeDef *adc){
//     ADC_startConversion(adc);
//     while(!ADC_isReady()) {}
//     uint16_t sample = ADC_readValue();
//     uint8_t intensity = (uint8_t)(((uint32_t)sample * 100U) / 4095U);
//     PWM_SetDuty(intensity);
// }

// uint8_t controleurled_buttonPressed(GPIO_TypeDef* port, uint8_t pin){
// 	return GPIO_readPin(port,pin);
// }
