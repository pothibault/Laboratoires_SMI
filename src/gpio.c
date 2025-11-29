#include "Includes/gpio.h"
#include "Includes/macros_utiles.h"
#include "stdint.h"
#include <stdbool.h>

void GPIO_initPin(GPIO_TypeDef* port, uint8_t pin, GPIONode_t mode) {

    // Activation horloge du port
    if      (port == GPIOA) RCC->AHB1ENR |= BIT0;
    else if (port == GPIOB) RCC->AHB1ENR |= BIT1;
    else if (port == GPIOC) RCC->AHB1ENR |= BIT2;
    else if (port == GPIOD) RCC->AHB1ENR |= BIT3;
    else if (port == GPIOE) RCC->AHB1ENR |= BIT4;
    else if (port == GPIOF) RCC->AHB1ENR |= BIT5;
    else if (port == GPIOG) RCC->AHB1ENR |= BIT6;
    else if (port == GPIOH) RCC->AHB1ENR |= BIT7;


    // Reset les 2 bits de MODER
    port->MODER &= ~(3U << (pin * 2));
    
    // Set le mode
    if (mode == GPIO_OUTPUT) {
        port->MODER |= (1U << (pin * 2));
    } else if (mode == GPIO_ANALOG) {
        port->PUPDR &= ~(3U << (pin * 2));
        port->MODER |= (3U << (pin * 2));
    } else if (mode == GPIO_AF) {
        port->MODER |= (2U << (pin * 2));
    } else if (mode == GPIO_INPUT) {
        port->PUPDR &= ~(3U << (pin * 2));
        port->PUPDR |=  (2U << (pin * 2));
    }
}

int GPIO_readPin(GPIO_TypeDef* port, uint8_t pin) {
    return ((port->IDR & (1U << pin)) ? 1 : 0);
}

void GPIO_writePin(GPIO_TypeDef* port, uint8_t pin, uint8_t bit) {
    if (bit) port->ODR |= (1U << pin);
    else     port->ODR &= ~(1U << pin);
}

void GPIO_setAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num) {
    volatile uint32_t *AFR = (pin < 8) ? &port->AFR[0] : &port->AFR[1];
    uint8_t idx = (pin < 8) ? pin : (pin - 8);
    *AFR &= ~(0xFU << (idx * 4));
    *AFR |=  ((uint32_t)(af_num & 0xF) << (idx * 4));
}

void GPIO_configPWMPad(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num) {
    GPIO_initPin(port, pin, GPIO_AF);
    GPIO_setAF(port, pin, af_num);
    port->OTYPER  &= ~(1U << pin);
    port->OSPEEDR |=  (2U << (pin * 2));
    port->PUPDR   &= ~(3U << (pin * 2));
}

void GPIO_configAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num,
                   GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd)
{
    GPIO_initPin(port, pin, GPIO_AF);
    GPIO_setAF(port, pin, af_num);
    
    if (otype == GPIO_OT_PP) port->OTYPER &= ~(1U << pin);
    else                     port->OTYPER |=  (1U << pin);

    port->OSPEEDR &= ~(3U << (pin * 2));
    port->OSPEEDR |=  ((uint32_t)speed << (pin * 2));

    port->PUPDR &= ~(3U << (pin * 2));
    port->PUPDR |=  ((uint32_t)pupd << (pin * 2));
}

void GPIO_configOutput(GPIO_TypeDef* port, uint8_t pin,
                       GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd)
{
    GPIO_initPin(port, pin, GPIO_OUTPUT);
    
    if (otype == GPIO_OT_PP) port->OTYPER &= ~(1U << pin);
    else                     port->OTYPER |=  (1U << pin);

    port->OSPEEDR &= ~(3U << (pin * 2));
    port->OSPEEDR |=  ((uint32_t)speed << (pin * 2));

    port->PUPDR &= ~(3U << (pin * 2));
    port->PUPDR |=  ((uint32_t)pupd << (pin * 2));
}

bool GPIO_checkAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af_num,
                  GPIO_OType_t otype, GPIO_Speed_t speed, GPIO_Pupd_t pupd)
{
    // Vérif MODER = Alternate function (10b)
    uint32_t mode = (port->MODER >> (pin * 2)) & 0x3u;
    if (mode != GPIO_AF) {
        return false;
    }

    // Vérif AFRx = af_num
    uint8_t idx = (pin < 8) ? pin : (pin - 8);
    uint32_t afr = (pin < 8) ? port->AFR[0] : port->AFR[1];
    uint32_t af_read = (afr >> (idx * 4)) & 0xFu;
    if (af_read != (af_num & 0xFu)) {
        return false;
    }

    // Vérif OTYPER
    uint32_t ot = (port->OTYPER >> pin) & 0x1u;
    if (ot != (uint32_t)otype) {
        return false;
    }

    // Vérif OSPEEDR
    uint32_t spd = (port->OSPEEDR >> (pin * 2)) & 0x3u;
    if (spd != (uint32_t)speed) {
        return false;
    }

    // Vérif PUPDR
    uint32_t pu = (port->PUPDR >> (pin * 2)) & 0x3u;
    if (pu != (uint32_t)pupd) {
        return false;
    }

    return true;
}