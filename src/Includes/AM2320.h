#ifndef AM2320_H
#define AM2320_H

#include <stdint.h>
#include <stdbool.h>

#define AM2320_I2C_ADDR   0x5C  // 7-bit address of AM2320

typedef struct {
    float temperature_c;
    float humidity_rh;
} am2320_data_t;

bool am2320_read(am2320_data_t *out);


#endif