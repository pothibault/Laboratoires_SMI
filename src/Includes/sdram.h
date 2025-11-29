#ifndef SDRAM_H_
#define SDRAM_H_

#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx.h"
#include <stdbool.h>


// Adresse de base SDRAM Bank 2 (FMC)
#define SDRAM_BANK2_BASE    ((uint32_t)0xD0000000UL)
#define SDRAM_SIZE_BYTES    (8u * 1024u * 1024u) //64 Mbit = 8 MiB

void SDRAM_Init(void);

void     SDRAM_Write16(uint32_t addr, uint16_t value);
uint16_t SDRAM_Read16(uint32_t addr);

void SDRAM_WriteBuffer16(uint32_t addr, const uint16_t *src, size_t count);

#endif
