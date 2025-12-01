#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

void I2C1_init(uint32_t pclk_hz, uint32_t speed_hz);

bool I2C1_write(uint8_t addr7, const uint8_t *data, uint8_t len);
bool I2C1_read(uint8_t addr7, uint8_t *data, uint8_t len);

#endif