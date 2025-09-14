#include "Includes/delai.h"
#include "Includes/macros_utiles.h"
#include "stm32f4xx.h"

//Définition des adresses des registres en Unsigned Long
#define SYST_CSR  (*(volatile uint32_t*)0xE000E010UL) //SysTick Control and Status Register
#define SYST_RVR  (*(volatile uint32_t*)0xE000E014UL) //SysTick Reload Value Register
#define SYST_CVR  (*(volatile uint32_t*)0xE000E018UL) //SysTick Current Value Register

static volatile uint32_t compteur_ms = 0;

void InitSysTick_1ms(uint32_t clk_hz) {
	uint32_t reload = (clk_hz/1000UL) - 1UL;

	if (reload < 1UL) reload = 1UL;
	if (reload > 0xFFFFFFU) reload = 0xFFFFFFU;

	SYST_RVR = reload;
	SYST_CVR = 0;

	//CLKSOURCE=1 (BIT2), TICKINT=1 (BIT1), ENABLE=1 (BIT0)
	SYST_CSR = BIT2 | BIT1 | BIT0;   // 0x00000007
}

uint32_t millis(void) {
	return compteur_ms;
}

void delay_ms_blocking(uint32_t ms) {
	uint32_t t0 = millis();

	// __WFI() Dort en attendant une interruption
	while ((uint32_t)(millis() - t0) < ms) {
		__WFI();
	}
}

void timer_start(timer_t *t) {
	t->timer_ms = millis();
}

bool timer_expired(timer_t *t, uint32_t delay_ms) {
	uint32_t now = millis();

	if ((uint32_t)(now - t->timer_ms) >= delay_ms ) {
		return true;
	}

	return false;
}


/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	compteur_ms++;
}
