#include "Includes/pwm.h"

static inline volatile uint32_t* pwm_ccr_ptr(TIM_TypeDef *tim, uint8_t ch) {
    switch (ch) {
        case 1: return &tim->CCR1;
        case 2: return &tim->CCR2;
        case 3: return &tim->CCR3;
        case 4: return &tim->CCR4;
        default: return 0;
    }
}

static inline void pwm_cfg_channel_mode(TIM_TypeDef *tim, uint8_t ch) {
    // PWM mode 1 (OCxM=110) + preload CCR (OCxPE=1)
    if (ch == 1) {
        tim->CCMR1 &= ~(0x7U << 4);
        tim->CCMR1 |=  (6U   << 4) | TIM_CCMR1_OC1PE;
    } else if (ch == 2) {
        tim->CCMR1 &= ~(0x7U << 12);
        tim->CCMR1 |=  (6U   << 12) | TIM_CCMR1_OC2PE;
    } else if (ch == 3) {
        tim->CCMR2 &= ~(0x7U << 4);
        tim->CCMR2 |=  (6U   << 4) | TIM_CCMR2_OC3PE;
    } else if (ch == 4) {
        tim->CCMR2 &= ~(0x7U << 12);
        tim->CCMR2 |=  (6U   << 12) | TIM_CCMR2_OC4PE;
    }
}

static inline void pwm_enable_channel_output(TIM_TypeDef *tim, uint8_t ch) {
    // CCxE bits: ch1->bit0, ch2->bit4, ch3->bit8, ch4->bit12
    uint32_t shift = (uint32_t)(ch - 1U) * 4U;
    tim->CCER &= ~(1U << shift);
    tim->CCER |=  (1U << shift); // active, polarité par défaut active high
}

void PWM_InitTimer(PWM_Handle_t *hpwm, TIM_TypeDef *tim, uint8_t channel, uint32_t timclk_hz) {
    hpwm->tim       = tim;
    hpwm->channel   = channel;
    hpwm->timclk_hz = timclk_hz;

    tim->CR1   = 0;
    tim->CCMR1 = 0;
    tim->CCMR2 = 0;
    tim->CCER  = 0;

    pwm_cfg_channel_mode(tim, channel);
    pwm_enable_channel_output(tim, channel);

    tim->CR1 |= TIM_CR1_ARPE; // preload ARR
    tim->EGR  = TIM_EGR_UG;   // sync registres d’ombre
}

uint32_t PWM_SetFrequency(PWM_Handle_t *hpwm, uint32_t frequency_hz) {
    TIM_TypeDef *tim = hpwm->tim;

    // Vise ~1 MHz de clock compteur
    uint32_t psc = (hpwm->timclk_hz / 1000000U);
    if (psc == 0) psc = 1;
    psc -= 1U;
    uint32_t cnt_clk = hpwm->timclk_hz / (psc + 1U);

    uint32_t arr = (cnt_clk / frequency_hz);
    if (arr == 0) arr = 1;
    arr -= 1U;

    // Conserver le duty courant si déjà configuré
    volatile uint32_t *ccr_ptr = pwm_ccr_ptr(tim, hpwm->channel);
    uint32_t old_arr = tim->ARR;
    uint32_t duty_percent = 0U;
    if (old_arr > 0U && ccr_ptr) {
        uint64_t num = (uint64_t)(*ccr_ptr) * 100ULL;
        duty_percent = (uint32_t)(num / (old_arr + 1ULL));
        if (duty_percent > 100U) duty_percent = 100U;
    }

    tim->CR1 &= ~TIM_CR1_CEN;
    tim->PSC  = psc;
    tim->ARR  = arr;

    if (ccr_ptr) {
        uint32_t ccr = (uint32_t)(((uint64_t)(arr + 1ULL) * duty_percent) / 100ULL);
        if (ccr > arr) ccr = arr;
        *ccr_ptr = ccr;
    }

    tim->EGR  = TIM_EGR_UG;
    tim->CR1 |= TIM_CR1_CEN;
    return arr;
}

void PWM_SetDuty(PWM_Handle_t *hpwm, uint8_t duty_percent) {
    if (duty_percent > 100U) duty_percent = 100U;

    TIM_TypeDef *tim = hpwm->tim;
    volatile uint32_t *ccr_ptr = pwm_ccr_ptr(tim, hpwm->channel);
    if (!ccr_ptr) return;

    uint32_t arr = tim->ARR;
    uint32_t ccr = (uint32_t)(((uint64_t)(arr + 1ULL) * duty_percent) / 100ULL);
    if (ccr > arr) ccr = arr;

    *ccr_ptr = ccr;
}

void PWM_Start(PWM_Handle_t *hpwm) {
    hpwm->tim->EGR  = TIM_EGR_UG;
    hpwm->tim->CR1 |= TIM_CR1_CEN;
}

void PWM_Stop(PWM_Handle_t *hpwm) {
    hpwm->tim->CR1 &= ~TIM_CR1_CEN;
}
