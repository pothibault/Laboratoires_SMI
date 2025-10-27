#pragma once
#include <stddef.h>
#include <stdint.h>
#include "stm32f4xx.h"

void SPI_Init_ForLCD(void);
void SPI_Transmit(const uint8_t *buf, size_t len);
