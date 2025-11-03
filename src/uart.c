#include "Includes/gpio.h"
#include "Includes/macros_utiles.h"
#include "stm32f4xx.h"
#include "uart.h"

#define UART5_RX_BUF_SIZE 128

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

/* --- Calcule BRR pour oversampling x16 --- */
static uint32_t compute_brr(uint32_t clk, uint32_t baud) {
    double usartdiv = (double)clk / (16.0 * (double)baud);
    uint32_t mant = (uint32_t)usartdiv;
    uint32_t frac = (uint32_t)((usartdiv - mant) * 16.0 + 0.5);
    if (frac >= 16) { mant++; frac = 0; }
    return (mant << 4) | (frac & 0xF);
}

void UART5_init(uint32_t apb1_clk_hz, uint32_t baud)
{
    /* 1. Configurer GPIO : PC12 = TX, PD2 = RX */
    GPIO_configAF(GPIOC, 12, 8, GPIO_OT_PP, GPIO_SPEED_VHIGH, GPIO_PUPD_NONE);
    GPIO_configAF(GPIOD,  2, 8, GPIO_OT_PP, GPIO_SPEED_VHIGH, GPIO_PUPD_NONE);

    /* 2. Activer horloge UART5 */
    RCC->APB1ENR |= BIT20;  // UART5EN

    /* 3. Configurer UART5 */
    UART5->CR1 &= ~USART_CR1_UE;      // Disable UART pendant config
    UART5->CR2 &= ~USART_CR2_STOP;    // 1 stop bit
    UART5->CR1 &= ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS |
                    USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE);
    UART5->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE); // 8 bits, no parity
    UART5->BRR = compute_brr(apb1_clk_hz, baud);
    UART5->CR1 |= USART_CR1_UE;       // Enable UART

    NVIC_EnableIRQ(UART5_IRQn);
    NVIC_SetPriority(UART5_IRQn, 5);
}

/* --- Écriture bloquante --- */
void UART5_putc(uint8_t c)
{
    while (!(UART5->SR & USART_SR_TXE));
    UART5->DR = c;
}

/* --- Envoi de chaîne --- */
void UART5_sendString(const char *s)
{
    while (*s) UART5_putc((uint8_t)*s++);
}

/* --- Lecture non bloquante (FIFO) --- */
int UART5_getc_nonblocking(uint8_t *c)
{
    return rx_pop(c);
}

/* --- Handler d’interruption --- */
void UART5_IRQHandler(void)
{
    if (UART5->SR & USART_SR_RXNE) {
        uint8_t b = (uint8_t)(UART5->DR & 0xFF);
        rx_push(b);
    }
}
