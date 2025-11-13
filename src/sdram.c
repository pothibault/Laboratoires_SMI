#include "Includes/sdram.h"
#include "Includes/gpio.h"
#include "Includes/delai.h"
#include <stdint.h>
#include <stdbool.h>

static inline void FMC_WaitWhileBusy(void) {
    while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY) {}
}

typedef enum {
    SDCMR_MODE_CLK_ENABLE   = 0x1u,
    SDCMR_MODE_PALL         = 0x2u,
    SDCMR_MODE_AUTOREFRESH  = 0x3u,
    SDCMR_MODE_LOAD_MODE    = 0x4u
} sdram_sdcmd_mode_t;

enum {
    SDCMR_MODE_Pos  = 0u,
	SDCMR_CTB1_Pos = 3u,
    SDCMR_CTB2_Pos  = 4u,
    SDCMR_NRFS_Pos  = 5u,
    SDCMR_MRD_Pos   = 9u
};

enum {
    SDCR_NC_Pos      = 0u,  
    SDCR_NR_Pos      = 2u,  
    SDCR_MWID_Pos    = 4u,  
    SDCR_NB_Pos      = 6u,  
    SDCR_CAS_Pos     = 7u,  
    SDCR_WP_Pos      = 9u,
    SDCR_SDCLK_Pos   = 10u, 
    SDCR_RBURST_Pos  = 12u, 
    SDCR_RPIPE_Pos   = 13u  
};

enum {
    SDTR_TMRD_Pos  = 0u,
    SDTR_TXSR_Pos  = 4u,
    SDTR_TRAS_Pos  = 8u,
    SDTR_TRC_Pos   = 12u,
    SDTR_TWR_Pos   = 16u,
    SDTR_TRP_Pos   = 20u,
    SDTR_TRCD_Pos  = 24u
};

static const uint32_t TMRD = 2u;
static const uint32_t TXSR = 3u;
static const uint32_t TRAS = 2u;
static const uint32_t TRC  = 3u;
static const uint32_t TWR  = 2u;
static const uint32_t TRP  = 1u;
static const uint32_t TRCD = 1u;

static const uint16_t SDRAM_MODE_REGISTER = 0x231u;


static const uint16_t SDRAM_REFRESH_COUNT = 543u; 

static inline void cfg(GPIO_TypeDef* port, uint8_t pin) {
    GPIO_configAF(port, pin, 12 /*AF12 FMC*/, GPIO_OT_PP, GPIO_SPEED_VHIGH, GPIO_PUPD_NONE);
}

static uint32_t sdram_refresh_count_from_hclk(uint32_t hclk_hz) {
    // SDCLK = HCLK/2 car SDCR.CLK = 10b
    uint32_t sdclk_hz = hclk_hz / 2u;
    // tREF/rows = 64ms / 4096 = 15.625 µs
    // COUNT = round( sdclk_hz * 64ms / 4096 ) - 20
    uint64_t numer = (uint64_t)sdclk_hz * 64u + (4096u*1000u)/2u; // arrondi
    uint32_t cycles = (uint32_t)(numer / (4096u * 1000u));
    return (cycles - 20u) & 0x1FFFu;
}



void SDRAM_Init(void)
{
    // 1. Horloge FMC
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

    cfg(GPIOD,14); cfg(GPIOD,15); cfg(GPIOD,0);  cfg(GPIOD,1);
    cfg(GPIOE,7);  cfg(GPIOE,8);  cfg(GPIOE,9);  cfg(GPIOE,10);
    cfg(GPIOE,11); cfg(GPIOE,12); cfg(GPIOE,13); cfg(GPIOE,14);
    cfg(GPIOE,15); cfg(GPIOD,8);  cfg(GPIOD,9);  cfg(GPIOD,10);

    // A0..A11
    cfg(GPIOF,0);  cfg(GPIOF,1);  cfg(GPIOF,2);  cfg(GPIOF,3);
    cfg(GPIOF,4);  cfg(GPIOF,5);  cfg(GPIOF,12); cfg(GPIOF,13);
    cfg(GPIOF,14); cfg(GPIOF,15); cfg(GPIOG,0);  cfg(GPIOG,1);

    // BA / control
    cfg(GPIOF,11);   // SDNRAS
    cfg(GPIOG,15);   // SDNCAS
    cfg(GPIOB,5);    // SDCKE1
    cfg(GPIOB,6);    // SDNE1
    cfg(GPIOG,4);    // BA0
    cfg(GPIOG,5);    // BA1
    cfg(GPIOE,0);    // NBL0
    cfg(GPIOE,1);    // NBL1
    cfg(GPIOG,8);    // SDCLK
    cfg(GPIOC,0);    // SDNWE

    // 3. Configuration commune SDCR pour Bank1 & Bank2
    uint32_t sdcr_common =
        (1u << SDCR_RPIPE_Pos) |      // RPIPE = 1 HCLK
        (2u << SDCR_SDCLK_Pos) |      // SDCLK = HCLK/2
        (0u << SDCR_RBURST_Pos) |     // no read burst
        (3u << SDCR_CAS_Pos)    |     // CAS = 3
        (1u << SDCR_NB_Pos)     |     // 4 internal banks
        (1u << SDCR_MWID_Pos)   |     // 16-bit
        (1u << SDCR_NR_Pos)     |     // 12 row bits
        (0u << SDCR_NC_Pos)     |     // 8 column bits
        (0u << SDCR_WP_Pos);          // no write protect

    FMC_Bank5_6->SDCR[0] = sdcr_common;  // Bank1
    FMC_Bank5_6->SDCR[1] = sdcr_common;  // Bank2
    FMC_WaitWhileBusy();

    // 4. Timings : mêmes valeurs pour les deux banks
    uint32_t sdtr0 =
        ((TRP -1u) << SDTR_TRP_Pos) |
        ((TRC -1u) << SDTR_TRC_Pos);

    uint32_t sdtr1 =
        ((TMRD-1u) << SDTR_TMRD_Pos) |
        ((TXSR-1u) << SDTR_TXSR_Pos) |
        ((TRAS-1u) << SDTR_TRAS_Pos) |
        ((TWR -1u) << SDTR_TWR_Pos)  |
        ((TRCD-1u) << SDTR_TRCD_Pos);

    FMC_Bank5_6->SDTR[0] = sdtr0;   
    FMC_Bank5_6->SDTR[1] = sdtr1;  
    FMC_WaitWhileBusy();

    uint32_t banks_mask = (1u << SDCMR_CTB1_Pos) | (1u << SDCMR_CTB2_Pos);

    // 5. Séquence d'initialisation SDRAM
    // 5.1 Clock enable
    FMC_Bank5_6->SDCMR =
        (SDCMR_MODE_CLK_ENABLE << SDCMR_MODE_Pos) |
        banks_mask;
    FMC_WaitWhileBusy();

    // petit délai > 100 µs
    delay_ms_blocking(2);

    // 5.2 Precharge all
    FMC_Bank5_6->SDCMR =
        (SDCMR_MODE_PALL << SDCMR_MODE_Pos) |
        banks_mask;
    FMC_WaitWhileBusy();

    // 5.3 Auto-refresh (8 cycles)
    FMC_Bank5_6->SDCMR =
        (SDCMR_MODE_AUTOREFRESH << SDCMR_MODE_Pos) |
        banks_mask |
        ((8u - 1u) << SDCMR_NRFS_Pos);
    FMC_WaitWhileBusy();

    // 5.4 Load mode register (0x231)
    FMC_Bank5_6->SDCMR =
        (SDCMR_MODE_LOAD_MODE << SDCMR_MODE_Pos) |
        banks_mask |
        ((uint32_t)SDRAM_MODE_REGISTER << SDCMR_MRD_Pos);
    FMC_WaitWhileBusy();

    // 5.5 Refresh rate
    FMC_Bank5_6->SDRTR = sdram_refresh_count_from_hclk(SystemCoreClock) << 1;
    FMC_WaitWhileBusy();
}


void SDRAM_Write16(uint32_t addr, uint16_t value)
{
    *(volatile uint16_t*)addr = value;
}

uint16_t SDRAM_Read16(uint32_t addr)
{
    return *(volatile uint16_t*)addr;
}

void SDRAM_WriteBuffer16(uint32_t addr, const uint16_t *src, size_t count)
{
    volatile uint16_t *dst = (volatile uint16_t*)addr;
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}
