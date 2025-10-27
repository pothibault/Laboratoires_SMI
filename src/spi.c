#include "Includes/spi.h"
#include "Includes/gpio.h"


#define LCD_SCK_PORT   GPIOF
#define LCD_SCK_PIN    7
#define LCD_SCK_AF     5

#define LCD_MOSI_PORT  GPIOF
#define LCD_MOSI_PIN   9
#define LCD_MOSI_AF    5


void SPI_Init_ForLCD(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;

    //SPI en Alternate Function AF5, push-pull, high speed, no pull
    GPIO_configAF(LCD_SCK_PORT,  LCD_SCK_PIN,  LCD_SCK_AF,  GPIO_OT_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE);
    GPIO_configAF(LCD_MOSI_PORT, LCD_MOSI_PIN, LCD_MOSI_AF, GPIO_OT_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE);

    //SPI5: maître 8-bit, mode 0, NSS logiciel (SSM/SSI), prescaler PCLK/16 (sécuritaire pour démarrer)
    SPI5->CR1 = 0;
    SPI5->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;   // maître, NSS logiciel, slave selectionner a 1
    SPI5->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);             // CPOL=0, CPHA=0
    SPI5->CR1 &= ~(SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2);

    SPI5->CR1 |=  (SPI_CR1_BR_1 | SPI_CR1_BR_0);                    // BR=0b011 => PCLK/16 (augmente plus tard si OK)

    SPI5->CR1 |= SPI_CR1_SPE;                                // Enable SPI
}

void SPI_Transmit(const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        while (!(SPI5->SR & SPI_SR_TXE)) {} //SPI_SR_TXE: registre DR pret a recevoir valeur, 
        *(volatile uint8_t*)&SPI5->DR = buf[i];
    }
    while (SPI5->SR & SPI_SR_BSY) {} //attendre que la trame soit finie
    (void)SPI5->SR; (void)SPI5->DR; // lecture du slave
}
