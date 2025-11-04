#ifndef AFFICHAGE_H_
#define AFFICHAGE_H_

#include <stdint.h>
#include "Includes/lcd_driver.h"
#include "Includes/spi.h"
#include "Includes/uart.h"

// Couleurs par défaut
#define COLOR_TEXT_DEFAULT  0xFFFF  // blanc
#define COLOR_BG_DEFAULT    0x0000  // noir

void Affichage_Init(void);
void Affichage_Update(void);

#endif 
