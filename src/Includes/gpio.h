#include "stm32f4xx.h"

// Enum pour modes pins
typedef enum {
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1,
    GPIO_AF = 2,
    GPIO_ANALOG = 3
} GPIONode_t;

// Nouveaux réglages "traits électriques"
typedef enum { GPIO_OT_PP = 0, GPIO_OT_OD = 1 } GPIO_OType_t; //mettre en mode push pull ou open drain
typedef enum { GPIO_SPEED_LOW = 0, GPIO_SPEED_MED = 1, GPIO_SPEED_HIGH = 2, GPIO_SPEED_VHIGH = 3 } GPIO_Speed_t; //vitesse de commutation
typedef enum { GPIO_PUPD_NONE = 0, GPIO_PUPD_PU = 1, GPIO_PUPD_PD = 2 } GPIO_Pupd_t; //mettre en pull up ou pull down

void GPIO_initPin(GPIO_TypeDef* port, uint8_t pin, GPIONode_t mode);

void GPIO_writePin(GPIO_TypeDef* port, uint8_t pin, uint8_t bit);

int GPIO_readPin(GPIO_TypeDef* port, uint8_t pin);

void GPIO_setAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num);

void GPIO_configPWM(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num);

void GPIO_configAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num,
                   GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd);

void GPIO_configOutput(GPIO_TypeDef* port, uint8_t pin,
                       GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd);
