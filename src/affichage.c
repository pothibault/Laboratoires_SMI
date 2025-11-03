#include "Includes/affichage.h"
#include "Includes/delai.h"
#include "Includes/gpio.h"
#include "Includes/spi.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// ---------------------- FIFO interne -----------------------
#define UART_RX_FIFO_SIZE 128
static volatile uint8_t uart_rx_fifo[UART_RX_FIFO_SIZE];
static volatile uint16_t fifo_head = 0;
static volatile uint16_t fifo_tail = 0;

static int fifo_is_empty(void) { return fifo_head == fifo_tail; }
static int fifo_is_full(void) { return ((fifo_head + 1) % UART_RX_FIFO_SIZE) == fifo_tail; }

static void fifo_push(uint8_t data) {
    if (!fifo_is_full()) {
        uart_rx_fifo[fifo_head] = data;
        fifo_head = (fifo_head + 1) % UART_RX_FIFO_SIZE;
    }
}

static uint8_t fifo_pop(void) {
    uint8_t data = 0;
    if (!fifo_is_empty()) {
        data = uart_rx_fifo[fifo_tail];
        fifo_tail = (fifo_tail + 1) % UART_RX_FIFO_SIZE;
    }
    return data;
}

// ---------------------- Variables globales locales -----------------------
static uint16_t cursorX = 0;
static uint16_t cursorY = 0;
static uint16_t textColor = COLOR_TEXT_DEFAULT;
static uint16_t bgColor   = COLOR_BG_DEFAULT;

#define LCD_WIDTH   240
#define LCD_HEIGHT  320
#define CHAR_W      16
#define CHAR_H      16

// ---------------------- Prototypes internes -----------------------
static uint16_t RGB888_to_RGB565(uint8_t r, uint8_t g, uint8_t b);
static void Affichage_NewLine(void);
static void Affichage_Caractere(uint8_t c);
static void Affichage_ParseCommande(const char* cmd);

// -------------------------------------------------------------
// Initialisation de l’affichage et du LCD
// -------------------------------------------------------------
void Affichage_Init(void)
{
    LCD_InitGPIO();
    SPI_Init_ForLCD();
    LCD_InitSerialInterface();

    LCD_CopyColorToFrameBuffer(bgColor);
    LCD_TransmitFrameBuffer();

    cursorX = 0;
    cursorY = 0;
}

// -------------------------------------------------------------
// Routine d’interruption UART (à appeler depuis ISR)
// -------------------------------------------------------------
void Affichage_UART_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE) { // exemple : USART2
        uint8_t c = USART2->DR;
        fifo_push(c);
    }
}

// -------------------------------------------------------------
// Boucle d’affichage principale (à appeler périodiquement)
// -------------------------------------------------------------
void Affichage_Update(void)
{
    static char cmdBuffer[16];
    static uint8_t cmdIndex = 0;
    static uint8_t inCmd = 0;

    while (!fifo_is_empty()) {
        uint8_t c = fifo_pop();

        if (c == '\n') {
            Affichage_NewLine();
        }
        else if (c == '\r') {
            cursorX = 0;
        }
        else {
            if (!inCmd) {
                if (c == 'S' || c == 'B') {
                    inCmd = 1;
                    cmdIndex = 0;
                    cmdBuffer[cmdIndex++] = c;
                } else {
                    Affichage_Caractere(c);
                }
            } else {
                cmdBuffer[cmdIndex++] = c;
                if (cmdIndex >= 8) {
                    cmdBuffer[cmdIndex] = '\0';
                    Affichage_ParseCommande(cmdBuffer);
                    inCmd = 0;
                    cmdIndex = 0;
                }
            }
        }
    }
}

// -------------------------------------------------------------
// Convertit RGB888 → RGB565
// -------------------------------------------------------------
static uint16_t RGB888_to_RGB565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t r5 = (r >> 3) & 0x1F;
    uint16_t g6 = (g >> 2) & 0x3F;
    uint16_t b5 = (b >> 3) & 0x1F;
    return (r5 << 11) | (g6 << 5) | b5;
}

// -------------------------------------------------------------
// Affiche un caractère sur le LCD
// -------------------------------------------------------------
static void Affichage_Caractere(uint8_t c)
{
    LCD_WriteChar(c, bgColor, textColor, cursorX, cursorY);
    cursorX += CHAR_W;

    if (cursorX + CHAR_W >= LCD_WIDTH)
        Affichage_NewLine();
}

// -------------------------------------------------------------
// Passe à la ligne suivante
// -------------------------------------------------------------
static void Affichage_NewLine(void)
{
    cursorX = 0;
    cursorY += CHAR_H;

    if (cursorY + CHAR_H >= LCD_HEIGHT)
        cursorY = 0; // retourne en haut (simple)
}

// -------------------------------------------------------------
// Interprète commandes SCxxxxxx / BCxxxxxx
// -------------------------------------------------------------
static void Affichage_ParseCommande(const char* cmd)
{
    uint32_t rgb = strtol(&cmd[2], NULL, 16);
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;

    if (strncmp(cmd, "SC", 2) == 0 || strncmp(cmd, "sc", 2) == 0) {
        textColor = RGB888_to_RGB565(r, g, b);
    }
    else if (strncmp(cmd, "BC", 2) == 0 || strncmp(cmd, "bc", 2) == 0) {
        bgColor = RGB888_to_RGB565(r, g, b);
        LCD_CopyColorToFrameBuffer(bgColor);
        LCD_TransmitFrameBuffer();
    }
}
