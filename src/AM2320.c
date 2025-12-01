#include "Includes/am2320.h"
#include "Includes/i2c.h"
#include "Includes/delai.h"

#include <stddef.h>

//dummy write
static void am2320_wake(void)
{
    (void)I2C1_write(AM2320_I2C_ADDR, NULL, 0);
    delay_ms_blocking(2);
}

bool am2320_read(am2320_data_t *out) {

    if (out == NULL) return false;

    uint8_t cmd[3] = {0x03, 0x00, 0x04}; //function code: read registers, starting register address, number of registers (4 bytes: H_H, H_L, T_H, T_L)

    am2320_wake();

    if (!I2C1_write(AM2320_I2C_ADDR, cmd, sizeof(cmd))) {
        return false;
    }

    delay_ms_blocking(2);

    uint8_t buf[8];

    if (!I2C1_read(AM2320_I2C_ADDR, buf, sizeof(buf))) {
        return false;
    }

    if (buf[0] != 0x03 || buf[1] != 0x04) {
        return false;
    }

    uint16_t raw_humidity = ((uint16_t)buf[2] << 8) | buf[3];
    uint16_t raw_temperature = ((uint16_t)buf[4] << 8) | buf[5];

    out->humidity_rh   = (float)raw_humidity / 10.0f;
    out->temperature_c = (float)( (int16_t)raw_temperature ) / 10.0f;

    return true;

}

