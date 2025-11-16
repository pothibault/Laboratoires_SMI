#include "Includes/gpio.h"
#include "Includes/macros_utiles.h"
#include "stm32f4xx.h"
#include "Includes/uart.h"

#define UART5_RX_BUF_SIZE 128

volatile int UART_DelayX = 0;
static volatile uint8_t rx_buf[UART5_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

/* --- Fonctions internes FIFO --- */
static inline void rx_push(uint8_t b) {
    uint16_t next = (rx_head + 1) % UART5_RX_BUF_SIZE;
    if (next != rx_tail) {
        rx_buf[rx_head] = b;
        rx_head = next;
    }
}

static inline int rx_pop(uint8_t *b) {
    if (rx_head == rx_tail) return 0;
    *b = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % UART5_RX_BUF_SIZE;
    return 1;
}


static uint32_t compute_brr(uint32_t clk, uint32_t baud) {
    double usartdiv = (double)clk / (16.0 * (double)baud);
    uint32_t mant = (uint32_t)usartdiv;
    uint32_t frac = (uint32_t)((usartdiv - mant) * 16.0 + 0.5);
    if (frac >= 16) { mant++; frac = 0; }
    return (mant << 4) | (frac & 0xF);
}

#include "stm32f4xx.h"
#include <stdint.h>

void UART5_init(uint32_t pclk1, uint32_t baudrate)
{
    // Horloge
    RCC->APB1ENR |= RCC_APB1ENR_UART5EN;

    // GPIO: TX=PC12, RX=PD2
    GPIO_configAF(GPIOC, 12, 8, GPIO_OT_PP, GPIO_SPEED_HIGH, GPIO_PUPD_NONE); // TX
    GPIO_configAF(GPIOD,  2, 8, GPIO_OT_PP, GPIO_SPEED_HIGH, GPIO_PUPD_PU);   // RX (pull-up)

    // Désactiver avant config
    UART5->CR1 &= ~USART_CR1_UE;

    // Baudrate (oversampling x16)
    UART5->BRR = compute_brr(pclk1, baudrate);

    // 1 stop bit
    UART5->CR2 &= ~USART_CR2_STOP; // 00b = 1 stop

    // Pas de DMA, pas de flow control HW
    UART5->CR3 &= ~(USART_CR3_DMAT | USART_CR3_DMAR | USART_CR3_RTSE | USART_CR3_CTSE);

    
    //TE/RE activés
    //RXNEIE actif
    //M=1 (9 bits -> 8 données + parité)
    //PCE=1 (parité activée)
    //PS=0 (parité paire)
    UART5->CR1 =
        USART_CR1_TE |
        USART_CR1_RE |
        USART_CR1_RXNEIE |
        USART_CR1_M   |      
        USART_CR1_PCE;         

    // Activer l’UART
    UART5->CR1 |= USART_CR1_UE;

    // NVIC
    NVIC_SetPriority(UART5_IRQn, 1);
    NVIC_EnableIRQ(UART5_IRQn);
}


void UART5_putc(uint8_t c)
{
    while (!(UART5->SR & USART_SR_TXE));
    UART5->DR = c;
}

void UART5_sendString(const char *s)
{
    while (*s) UART5_putc((uint8_t)*s++);
}

int UART5_getc_nonblocking(uint8_t *c)
{
    return rx_pop(c);
}

void UART5_IRQHandler(void)
{

    // Toggle PG13 au début de l'ISR pour labo 4 partie 2.1
    static bool led_state = false;
    GPIO_writePin(GPIOG, 13, led_state);
    led_state = !led_state;
    // Boucle d’attente artificielle labo4 partie 2.1
    for (volatile int i = 0; i < UART_DelayX; i++);

    uint32_t sr = UART5->SR;              //lire SR

    if (sr & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {
        volatile uint8_t dummy = UART5->DR;  //lire DR pour clear les flags
        (void)dummy;
        return;                               // on ignore l’octet en erreur
    }

    #ifdef UART_DIRECT_LCD
    // Labo4 partie 2.2
    uint8_t c = UART5->DR;
    LCD_WriteChar(c);      // Pas de FIFO
    #else
        if (sr & USART_SR_RXNE) {
        uint8_t b = (uint8_t)UART5->DR;      // parité retirée automatiquement
        rx_push(b);
    }
    #endif

}
