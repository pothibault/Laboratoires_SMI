#include "Includes/i2c.h"
#include "Includes/gpio.h"
#include <stddef.h>



static bool i2c1_wait_flag_sr1(uint32_t flag)
{
    uint32_t timeout = 1000000U;
    while (((I2C1->SR1) & flag) == 0U) {
        if (--timeout == 0U) return false;
    }
    return true;
}

static bool i2c1_wait_flag_sr2_clear(uint32_t flag)
{
    uint32_t timeout = 1000000U;
    while ((I2C1->SR2 & flag) != 0U) {
        if (--timeout == 0U) return false;
    }
    return true;
}


//Exemple: I2C1_init(36000000U, 100000U);  // PCLK1 = 72Mhz/2 = 36 MHz, I²C at 100 kHz
void I2C1_init(uint32_t pclk_hz, uint32_t speed_hz) {

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIO_configAF(GPIOB, 8, 4, GPIO_OT_OD, GPIO_SPEED_HIGH, GPIO_PUPD_PU); // SCL
    GPIO_configAF(GPIOB, 9, 4, GPIO_OT_OD, GPIO_SPEED_HIGH, GPIO_PUPD_PU); // SDA

    // Reset I2C1
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    // Configure frequency (CR2) in MHz
    uint32_t freq_mhz = pclk_hz / 1000000U;
    if (freq_mhz < 2U)  freq_mhz = 2U;
    if (freq_mhz > 50U) freq_mhz = 50U;
    I2C1->CR2 = freq_mhz;


    if (speed_hz == 0U) speed_hz = 100000U;
    uint32_t ccr = pclk_hz / (2U * speed_hz);
    if (ccr < 4U) ccr = 4U;
    I2C1->CCR = ccr;

    // Maximum rise time for standard mode: freq_MHz + 1
    I2C1->TRISE = freq_mhz + 1U;

    // Own address
    I2C1->OAR1 = (1U << 14);

    I2C1->CR1 |= I2C_CR1_PE;

}


bool I2C1_write(uint8_t addr7, const uint8_t *data, uint8_t len) {

    if (!i2c1_wait_flag_sr2_clear(I2C_SR2_BUSY)) return false;

    // Generate START
    I2C1->CR1 |= I2C_CR1_START;
    if (!i2c1_wait_flag_sr1(I2C_SR1_SB)) return false;

    // Send address
    I2C1->DR = (uint8_t)(addr7 << 1);
    if (!i2c1_wait_flag_sr1(I2C_SR1_ADDR)) return false;

    // Clear ADDR by reading SR1 then SR2
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    if (len == 0U) {
        I2C1->CR1 |= I2C_CR1_STOP;
        return true;
    }

    // Send data bytes
    for (uint8_t i = 0; i < len; ++i) {
        if (!i2c1_wait_flag_sr1(I2C_SR1_TXE)) return false;
        I2C1->DR = data[i];
    }

     // Wait for transfer finished
    if (!i2c1_wait_flag_sr1(I2C_SR1_BTF)) return false;

    I2C1->CR1 |= I2C_CR1_STOP;

    return true;

}


bool I2C1_read(uint8_t addr7, uint8_t *data, uint8_t len)
{
    if (len == 0U || data == NULL) return false;

    // Wait until bus not busy
    if (!i2c1_wait_flag_sr2_clear(I2C_SR2_BUSY)) return false;

    // Enable ACK by default
    I2C1->CR1 |= I2C_CR1_ACK;

    // Generate START
    I2C1->CR1 |= I2C_CR1_START;
    if (!i2c1_wait_flag_sr1(I2C_SR1_SB)) return false;

    // Send address
    I2C1->DR = (uint8_t)((addr7 << 1) | 0x01U);
    if (!i2c1_wait_flag_sr1(I2C_SR1_ADDR)) return false;

    if (len == 1U) {
        I2C1->CR1 &= ~I2C_CR1_ACK;      // NACK
        (void)I2C1->SR1; (void)I2C1->SR2; // clear ADDR
        I2C1->CR1 |= I2C_CR1_STOP;      // STOP

        if (!i2c1_wait_flag_sr1(I2C_SR1_RXNE)) return false;
        data[0] = (uint8_t)I2C1->DR;
    }
    else if (len == 2U) {
        I2C1->CR1 |= I2C_CR1_POS;
        (void)I2C1->SR1; (void)I2C1->SR2;

        I2C1->CR1 &= ~I2C_CR1_ACK;      // NACK
        if (!i2c1_wait_flag_sr1(I2C_SR1_BTF)) return false;

        I2C1->CR1 |= I2C_CR1_STOP;  

        data[0] = (uint8_t)I2C1->DR;
        data[1] = (uint8_t)I2C1->DR;
        I2C1->CR1 &= ~I2C_CR1_POS;      // Clear POS
    }
    else {
        (void)I2C1->SR1; (void)I2C1->SR2; // clear ADDR
        uint8_t remaining = len;

        while (remaining > 3U) {
            if (!i2c1_wait_flag_sr1(I2C_SR1_RXNE)) return false;
            *data++ = (uint8_t)I2C1->DR;
            remaining--;
        }

        if (!i2c1_wait_flag_sr1(I2C_SR1_BTF)) return false;

        I2C1->CR1 &= ~I2C_CR1_ACK;  // NACK for last byte

        *data++ = (uint8_t)I2C1->DR; // N-2
        I2C1->CR1 |= I2C_CR1_STOP;   // STOP
        *data++ = (uint8_t)I2C1->DR; // N-1

        if (!i2c1_wait_flag_sr1(I2C_SR1_RXNE)) return false;
        *data++ = (uint8_t)I2C1->DR; // N
    }

    return true;
}
