#include "Includes/affichage.h"
#include "Includes/delai.h"
#include "Includes/gpio.h"
#include "Includes/spi.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static uint16_t cursorX = 0;
static uint16_t cursorY = 0;
static uint16_t textColor = COLOR_TEXT_DEFAULT;
static uint16_t bgColor   = COLOR_BG_DEFAULT;

#define LCD_WIDTH   240
#define LCD_HEIGHT  320
#define CHAR_W      16
#define CHAR_H      16

static uint16_t RGB888_to_RGB565(uint8_t r, uint8_t g, uint8_t b);
static void Affichage_NewLine(void);
static void Affichage_Caractere(uint8_t c);
static void Affichage_ParseCommande(const char* cmd);

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

void Affichage_Update(void)
{
    static char cmd[10];            // "SC" + 6 hex + '\0'
    static uint8_t idx = 0;
    static uint8_t state = 0;       
    static char prefix = 0;    

    uint8_t c;

    while (UART5_getc_nonblocking(&c)) {
        if (c == '\n') { Affichage_NewLine(); continue; }
        if (c == '\r') { cursorX = 0;        continue; }

        switch (state) {
        case 0: // normal
            if (c=='S'||c=='s'||c=='B'||c=='b') {
                prefix = (c=='b'||c=='B') ? 'B' : 'S';
                state = 1;
            } else {
                Affichage_Caractere(c);
            }
            break;

        case 1: // vu S/B -> attendre C/c
            if (c=='C'||c=='c') {
                cmd[0] = prefix; cmd[1] = 'C'; idx = 2; state = 2;
            } else {
                // pas une commande imprimer la lettre précédente + le char courant
                Affichage_Caractere(prefix);
                Affichage_Caractere(c);
                state = 0; idx = 0;
            }
            break;

        case 2: // SC/BC -> lire 6 hex
            if ((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F')) {
                if (idx < 8) cmd[idx++] = c;
                if (idx == 8) {        
                    cmd[idx] = '\0';
                    Affichage_ParseCommande(cmd);
                    state = 0; idx = 0;
                } else {
                    state = 3;
                }
            } else {
                // invalide
                state = 0; idx = 0;
            }
            break;
        }
    }
}



static uint16_t RGB888_to_RGB565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t r5 = (r >> 3) & 0x1F;
    uint16_t g6 = (g >> 2) & 0x3F;
    uint16_t b5 = (b >> 3) & 0x1F;
    return (r5 << 11) | (g6 << 5) | b5;
}


static void Affichage_Caractere(uint8_t c)
{
    LCD_WriteChar(c, bgColor, textColor, cursorX, cursorY);
    cursorX += CHAR_W;

    if (cursorX + CHAR_W >= LCD_WIDTH)
        Affichage_NewLine();
}


static void Affichage_NewLine(void)
{
    cursorX = 0;
    cursorY += CHAR_H;

    if (cursorY + CHAR_H >= LCD_HEIGHT)
        cursorY = 0; // retourne en haut (simple)
}

static void Affichage_ParseCommande(const char* cmd)
{
    // cmd = "SCxxxxxx" ou "BCxxxxxx" (insensible à la casse)
    uint32_t rgb = (uint32_t)strtoul(cmd + 2, NULL, 16);
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8)  & 0xFF;
    uint8_t b =  rgb        & 0xFF;

    uint16_t c565 = RGB888_to_RGB565(r, g, b);

    if (cmd[0] == 'S' || cmd[0] == 's') {
        textColor = c565;                       // couleur du texte
    } else {
        bgColor = c565;                         // couleur de fond
        LCD_CopyColorToFrameBuffer(bgColor);
        LCD_TransmitFrameBuffer();
    }
}
