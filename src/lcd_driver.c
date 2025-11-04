

/*
 * ili9341_serial.c
 *
 *  Created on: Aug 17, 2024
 *      Author: Arnaud Lapierre
 */


#include "Includes/lcd_driver.h"
#include "Includes/gpio.h"
#include "Includes/delai.h"
#include "Includes/spi.h"

#define LCD_CSX_PORT   GPIOC
#define LCD_CSX_PIN    2
#define LCD_WRX_PORT   GPIOD
#define LCD_WRX_PIN    13



// variables locales
static uint16_t frame_buffer[LCD_BUF_LEN];
static uint8_t *frame_buffer_bytes = (uint8_t *)frame_buffer;
static uint8_t char_buff[CHAR_WIDTH_16*CHAR_HEIGHT_16*2] = {0};


// prototypes des fonctions statiques
static void ILI9341_send_command(uint8_t Command);
static void ILI9341_send_data(uint8_t Data);
static void LCD_CS_HIGH(void);
static void LCD_CS_LOW(void);
static void LCD_WRX_HIGH(void);
static void LCD_WRX_LOW(void);



// initialisation de l'ecran
// configuration de base + envoi des pixels par SPI
void LCD_InitSerialInterface(void)
{
	// ILI9341_Enable;
	LCD_CS_LOW();

	//SOFTWARE RESET
	ILI9341_send_command(0x01);

	// TODO utiliser votre module de delai pour 1 seconde
	delay_ms_blocking(1000);

	//POWER CONTROL A
	ILI9341_send_command(LCD_POWERA);
	ILI9341_send_data(0x39);
	ILI9341_send_data(0x2C);
	ILI9341_send_data(0x00);
	ILI9341_send_data(0x34);
	ILI9341_send_data(0x02);

	//POWER CONTROL B
	ILI9341_send_command(LCD_POWERB);
	ILI9341_send_data(0x00);
	ILI9341_send_data(0xC1);
	ILI9341_send_data(0x30);

	//DRIVER TIMING CONTROL A
	ILI9341_send_command(LCD_DTCA);
	ILI9341_send_data(0x85);
	ILI9341_send_data(0x00);
	ILI9341_send_data(0x78);

	//DRIVER TIMING CONTROL B
	ILI9341_send_command(LCD_DTCB);
	ILI9341_send_data(0x00);
	ILI9341_send_data(0x00);

	//POWER ON SEQUENCE CONTROL
	ILI9341_send_command(LCD_POWER_SEQ);
	ILI9341_send_data(0x64);
	ILI9341_send_data(0x03);
	ILI9341_send_data(0x12);
	ILI9341_send_data(0x81);

	//PUMP RATIO CONTROL
	ILI9341_send_command(LCD_PRC);
	ILI9341_send_data(0x20);

	//POWER CONTROL,VRH[5:0]
	ILI9341_send_command(LCD_POWER1);
	ILI9341_send_data(0x23);

	//POWER CONTROL,SAP[2:0];BT[3:0]
	ILI9341_send_command(LCD_POWER2);
	ILI9341_send_data(0x10);

	//VCM CONTROL
	ILI9341_send_command(LCD_VCOM1);
	ILI9341_send_data(0x3E);
	ILI9341_send_data(0x28);

	//VCM CONTROL 2
	ILI9341_send_command(LCD_VCOM2);
	ILI9341_send_data(0x86);

	//MEMORY ACCESS CONTROL
	ILI9341_send_command(LCD_MAC);
	ILI9341_send_data(0x48);

	//PIXEL FORMAT
	ILI9341_send_command(LCD_PIXEL_FORMAT);
	ILI9341_send_data(0x55); // 16 bits

	//FRAME RATIO CONTROL, STANDARD RGB COLOR
	ILI9341_send_command(LCD_FRMCTR1);
	ILI9341_send_data(0x00);
	ILI9341_send_data(0x18);

	//DISPLAY FUNCTION CONTROL
	ILI9341_send_command(LCD_DFC);
	ILI9341_send_data(0x08);
	ILI9341_send_data(0x82);
	ILI9341_send_data(0x27);

	//3GAMMA FUNCTION DISABLE
	ILI9341_send_command(LCD_3GAMMA_EN);
	ILI9341_send_data(0x00);

	//GAMMA CURVE SELECTED
	ILI9341_send_command(LCD_GAMMA);
	ILI9341_send_data(0x01);

	//POSITIVE GAMMA CORRECTION
	ILI9341_send_command(LCD_PGAMMA);
	ILI9341_send_data(0x0F);
	ILI9341_send_data(0x31);
	ILI9341_send_data(0x2B);
	ILI9341_send_data(0x0C);
	ILI9341_send_data(0x0E);
	ILI9341_send_data(0x08);
	ILI9341_send_data(0x4E);
	ILI9341_send_data(0xF1);
	ILI9341_send_data(0x37);
	ILI9341_send_data(0x07);
	ILI9341_send_data(0x10);
	ILI9341_send_data(0x03);
	ILI9341_send_data(0x0E);
	ILI9341_send_data(0x09);
	ILI9341_send_data(0x00);

	//NEGATIVE GAMMA CORRECTION
	ILI9341_send_command(LCD_NGAMMA);
	ILI9341_send_data(0x00);
	ILI9341_send_data(0x0E);
	ILI9341_send_data(0x14);
	ILI9341_send_data(0x03);
	ILI9341_send_data(0x11);
	ILI9341_send_data(0x07);
	ILI9341_send_data(0x31);
	ILI9341_send_data(0xC1);
	ILI9341_send_data(0x48);
	ILI9341_send_data(0x08);
	ILI9341_send_data(0x0F);
	ILI9341_send_data(0x0C);
	ILI9341_send_data(0x31);
	ILI9341_send_data(0x36);
	ILI9341_send_data(0x0F);

	//EXIT SLEEP
	ILI9341_send_command(LCD_SLEEP_OUT);

	// TODO utilisez votre module delai pour 120 ms
	delay_ms_blocking(120);

	//TURN ON DISPLAY
	ILI9341_send_command(LCD_DISPLAY_ON);

	//STARTING ROTATION
	ILI9341_send_command(0x36);

	// TODO utilisez votre module delai pour 2 ms
	delay_ms_blocking(2);

	ILI9341_send_data(0x80|0x08);
}


//Fonction pour �crire dans le frame buffer
void LCD_CopyColorToFrameBuffer(uint16_t color)
{
	// on s�pare le pixel en deux octets avant la copie dans le frame buffer (SPI est config pour octets)
	for(int i = 0; i < LCD_BUF_LEN_BYTES; i+=2)
	{
		frame_buffer_bytes[i] = (color >> 8); // MSB
		frame_buffer_bytes[i+1] = (color & 0xFF); //LSB
	}
}


// Fonction pour envoyer le frame buffer par SPI
void LCD_TransmitFrameBuffer(void)
{
	LCD_SetAddress(0, 0, ILI9341_LCD_PIXEL_WIDTH, ILI9341_LCD_PIXEL_HEIGHT);
	LCD_WRX_HIGH();
	LCD_CS_LOW();
	SPI_Transmit(frame_buffer_bytes, ILI9341_LCD_PIXEL_TOTAL_BYTES);
}


// Fonction pour �crire un caract�re en bitmap (Font16) au pixel _cursorX _cursorY
void LCD_WriteChar(uint8_t character, uint16_t backGroundColor, uint16_t charColor, uint16_t cursorX, uint16_t cursorY)
{
	uint32_t cursorXEnd = cursorX + CHAR_WIDTH_16 - 1;
	uint32_t cursorYend = cursorY + CHAR_HEIGHT_16 - 1;

	uint8_t  row    = 0;
	uint8_t  rowMSB = 0;
	uint8_t  rowLSB = 0;
	uint16_t row_index = 0;
	uint32_t tmp = 0;
	uint16_t color = 0;

	uint16_t asciiCharInFont16Array  = character - 32; // les caract�res commencent � 32 dans la table ascii
	uint16_t startingFontIndex       = asciiCharInFont16Array * CHAR_HEIGHT_16 * 2;

	// on boucle dans le font et on replace les valeurs de couleur dans char_buff

	// Chaque rang�e fait 2 bytes dans Font16
	for(int i = startingFontIndex; i < (startingFontIndex + (CHAR_HEIGHT_16 * 2)); i += 2)
	{
		// chaque rang�e fait 2 bytes
		rowMSB = Font16_Table[i];   // premier byte
		rowLSB = Font16_Table[i+1]; // deuxi�me byte

		// on passe bit par bit pour v�rifier s'il faut �crire un pixel de _charColor ou _backGroundColor
		for(int j = 0; j < CHAR_WIDTH_16; j++)
		{
			row = (j < BITS_IN_BYTE) ? rowMSB : rowLSB;

			// on calcule un bitmask qui passe dans la rang�e de gauche � droite pour trouver les pixels � �crire
			uint16_t mask = 1 << (BITS_IN_BYTE - 1 - (j % BITS_IN_BYTE));
			color = (row & mask) ? charColor : backGroundColor;

			// on �crit � coup de 2 bytes, car spi est config pour des octets
			tmp = j*2 + (CHAR_WIDTH_16 * row_index * 2);
			char_buff[tmp] = (color >> 8);
			char_buff[tmp + 1] = (uint8_t)color;
		}
		row_index++;
	}

	LCD_SetAddress(cursorX, cursorY, cursorXEnd, cursorYend);
	LCD_WRX_HIGH();
	LCD_CS_LOW();
	SPI_Transmit(char_buff, CHAR_WIDTH_16*CHAR_HEIGHT_16*2);
	LCD_CS_HIGH();
}


// Cette fonction delimite un rectangle d'�criture de pixels
// uint16_t X1 = colonne de debut
// uint16_t X2 = colonne de fin
// uint16_t Y1 = ligne de debut
// uint16_t Y2 = ligne de fin
void LCD_SetAddress(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
	ILI9341_send_command(LCD_COLUMN_ADDR);
	ILI9341_send_data(X1>>8); // MSB
	ILI9341_send_data(X1);
	ILI9341_send_data(X2>>8); // MSB
	ILI9341_send_data(X2);

    ILI9341_send_command(LCD_PAGE_ADDR);
    ILI9341_send_data(Y1>>8); // MSB
    ILI9341_send_data(Y1);
    ILI9341_send_data(Y2>>8); // MSB
    ILI9341_send_data(Y2);

    ILI9341_send_command(LCD_GRAM);
}


void LCD_InitGPIO(void)
{
	// TODO
	// Fonction pour initialiser les GPIO CS et WRX.
	// Utilisez votre pilote GPIO!
	GPIO_initPin(LCD_CSX_PORT,  LCD_CSX_PIN,  GPIO_OUTPUT);
    GPIO_initPin(LCD_WRX_PORT,  LCD_WRX_PIN,  GPIO_OUTPUT);

	GPIO_writePin(LCD_CSX_PORT, LCD_CSX_PIN, 1); // CSX inactif
    GPIO_writePin(LCD_WRX_PORT, LCD_WRX_PIN, 1); // WRX = DATA par défaut
}


///////////////Fonctions statiques ///////////////////

static void LCD_CS_HIGH(void)
{
	// TODO
	GPIO_writePin(LCD_CSX_PORT, LCD_CSX_PIN, 1);
}

static void LCD_CS_LOW(void)
{
	// TODO
	GPIO_writePin(LCD_CSX_PORT, LCD_CSX_PIN, 0);
}
static void LCD_WRX_HIGH(void)
{
	// TODO
	GPIO_writePin(LCD_WRX_PORT, LCD_WRX_PIN, 1); 
}
static void LCD_WRX_LOW(void)
{
	// 
	GPIO_writePin(LCD_WRX_PORT, LCD_WRX_PIN, 0); 
}


static void ILI9341_send_command(uint8_t Command)
{
    // LCD_WRX_LOW = envoyer une commande de configuration au ili9341
    LCD_WRX_LOW();

    // Reset LCD control line(/CS) et envoyer l'octet
    LCD_CS_LOW();
    SPI_Transmit(&Command, 1);
    LCD_CS_HIGH();
}


static void ILI9341_send_data(uint8_t Data)
{
    // WRX HIGH = envoyer une donnee
    LCD_WRX_HIGH();

    // Reset LCD control line(/CS) et envoyer l'octet
    LCD_CS_LOW();
    SPI_Transmit(&Data, 1);
    LCD_CS_HIGH();
}
