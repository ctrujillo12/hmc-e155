// flash/80mhz clk
// rcc peripheral clks
// pa6 alternate function
// tim16 pwm
// tim15 duration timer
// set_freq() func
// delay_ms() func

#include "timer.h"

#define __IO volatile

// ========================================================
// BIG CLOCK 
// ==========================================================

// This file configures the MCU for an 80 MHz system clock
#define SYSTEM_CLOCK_HZ 80000000UL


// ==========================================================
// BASE ADDRS
// ==========================================================

#define FLASH_BASE  0x40022000UL
#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x48000000UL

#define TIM15_BASE  0x40014000UL
#define TIM16_BASE  0x40014400UL


// =====================================================
// FLASH REGS
// ====================================================

typedef struct
{
    __IO uint32_t ACR;       // 0x00
    __IO uint32_t KEYR;      // 0x04
    __IO uint32_t OPTKEYR;   // 0x08
    __IO uint32_t SR;        // 0x0C
    __IO uint32_t CR;        // 0x10
} FLASH_TypeDef;

#define FLASH ((FLASH_TypeDef *) FLASH_BASE)


// ====================================================
// RCC REGS
// ===================================================

typedef struct
{
    __IO uint32_t CR;          // 0x00
    __IO uint32_t ICSCR;       // 0x04
    __IO uint32_t CFGR;        // 0x08
    __IO uint32_t PLLCFGR;     // 0x0C
    __IO uint32_t PLLSAI1CFGR; // 0x10

         uint32_t RESERVED0;   // 0x14

    __IO uint32_t CIER;        // 0x18
    __IO uint32_t CIFR;        // 0x1C
    __IO uint32_t CICR;        // 0x20

         uint32_t RESERVED1;   // 0x24

    __IO uint32_t AHB1RSTR;    // 0x28
    __IO uint32_t AHB2RSTR;    // 0x2C
    __IO uint32_t AHB3RSTR;    // 0x30

         uint32_t RESERVED2;   // 0x34

    __IO uint32_t APB1RSTR1;   // 0x38
    __IO uint32_t APB1RSTR2;   // 0x3C
    __IO uint32_t APB2RSTR;    // 0x40

         uint32_t RESERVED3;   // 0x44

    __IO uint32_t AHB1ENR;     // 0x48
    __IO uint32_t AHB2ENR;     // 0x4C
    __IO uint32_t AHB3ENR;     // 0x50

         uint32_t RESERVED4;   // 0x54

    __IO uint32_t APB1ENR1;    // 0x58
    __IO uint32_t APB1ENR2;    // 0x5C
    __IO uint32_t APB2ENR;     // 0x60

} RCC_TypeDef;

#define RCC ((RCC_TypeDef *) RCC_BASE)


// =====================================================
// GPIOA REGS
// ===================================================

typedef struct
{
    __IO uint32_t MODER;      // 0x00
    __IO uint32_t OTYPER;     // 0x04
    __IO uint32_t OSPEEDR;    // 0x08
    __IO uint32_t PUPDR;      // 0x0C
    __IO uint32_t IDR;        // 0x10
    __IO uint32_t ODR;        // 0x14
    __IO uint32_t BSRR;       // 0x18
    __IO uint32_t LCKR;       // 0x1C
    __IO uint32_t AFRL;       // 0x20
    __IO uint32_t AFRH;       // 0x24
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)


// ==================================================
// TIM15 REGS
// =====================================================

typedef struct
{
    __IO uint32_t CR1;        // 0x00
    __IO uint32_t CR2;        // 0x04
    __IO uint32_t SMCR;       // 0x08
    __IO uint32_t DIER;       // 0x0C
    __IO uint32_t SR;         // 0x10
    __IO uint32_t EGR;        // 0x14
    __IO uint32_t CCMR1;      // 0x18

         uint32_t RESERVED0;  // 0x1C

    __IO uint32_t CCER;       // 0x20
    __IO uint32_t CNT;        // 0x24
    __IO uint32_t PSC;        // 0x28
    __IO uint32_t ARR;        // 0x2C
    __IO uint32_t RCR;        // 0x30
    __IO uint32_t CCR1;       // 0x34
    __IO uint32_t CCR2;       // 0x38

         uint32_t RESERVED1;  // 0x3C
         uint32_t RESERVED2;  // 0x40

    __IO uint32_t BDTR;       // 0x44

} TIM15_TypeDef;

#define TIM15 ((TIM15_TypeDef *) TIM15_BASE)


// ==================================================
// TIM16 REGS
// ====================================================

typedef struct
{
    __IO uint32_t CR1;        // 0x00
    __IO uint32_t CR2;        // 0x04

         uint32_t RESERVED0;  // 0x08

    __IO uint32_t DIER;       // 0x0C
    __IO uint32_t SR;         // 0x10
    __IO uint32_t EGR;        // 0x14
    __IO uint32_t CCMR1;      // 0x18

         uint32_t RESERVED1;  // 0x1C

    __IO uint32_t CCER;       // 0x20
    __IO uint32_t CNT;        // 0x24
    __IO uint32_t PSC;        // 0x28
    __IO uint32_t ARR;        // 0x2C
    __IO uint32_t RCR;        // 0x30
    __IO uint32_t CCR1;       // 0x34

         uint32_t RESERVED2;  // 0x38
         uint32_t RESERVED3;  // 0x3C
         uint32_t RESERVED4;  // 0x40

    __IO uint32_t BDTR;       // 0x44

} TIM16_TypeDef;

#define TIM16 ((TIM16_TypeDef *) TIM16_BASE)


// ================================================
// TIMER SETTINGS
// ===================================================

// TIM16 PWM prescaler
// Counter frequency = 80 MHz / (25 + 1)
#define PWM_PSC 25U

// TIM15 prescaler
// 80 MHz / (7999 + 1) = 10 kHz
// Therefore 1 ms = 10 counter ticks
#define DELAY_PSC      7999U
#define COUNTS_PER_MS  10U


// =================================================
// PRIVATE INIT FNS
// =================================================


// ------------------------------------------------------------
// Configure for 80 MHz
// ------------------------------------------------------------

static void clock_init(void)
{
    // Set flash latency / configuration
    // This is adapted from the FLASH setup in the older project
    FLASH->ACR |= 0b100;
    FLASH->ACR |= (1U << 8);

    // Turn PLL off before changing its configuration
    RCC->CR &= ~(1U << 24);

    // Wait until PLLRDY goes low
    while (RCC->CR & (1U << 25))
    {
    }

    // PLL source = MSI
    RCC->PLLCFGR &= ~(3U << 0);
    RCC->PLLCFGR |=  (1U << 0);

    // PLLM = divide by 1
    RCC->PLLCFGR &= ~(7U << 4);

    // PLLN = 80
    RCC->PLLCFGR &= ~(0xFFU << 8);
    RCC->PLLCFGR |=  (80U << 8);

    // PLLR = divide by 4
    RCC->PLLCFGR &= ~(3U << 25);
    RCC->PLLCFGR |=  (1U << 25);

    // Enable PLLR output
    RCC->PLLCFGR |= (1U << 24);

    // Enable PLL
    RCC->CR |= (1U << 24);

    // Wait until PLL locks
    while (!(RCC->CR & (1U << 25)))
    {
    }

    // Select PLL as system clock
    RCC->CFGR &= ~(3U << 0);
    RCC->CFGR |=  (3U << 0);

    // Wait until PLL is actually the system clock
    while (((RCC->CFGR >> 2) & 3U) != 3U)
    {
    }
}


// -------------------------------------------------------
// Configure PA6 as TIM16_CH1 output
// -------------------------------------------------------

static void pwm_gpio_init(void)
{
    // Enable GPIOA clock
    RCC->AHB2ENR |= (1U << 0);

    // PA6 MODER = 10 -> alternate-function mode
    GPIOA->MODER &= ~(3U << (6U * 2U));
    GPIOA->MODER |=  (2U << (6U * 2U));

    // PA6 uses AFRL because it is pin 0-7
    // AF14 maps PA6 to TIM16_CH1
    GPIOA->AFRL &= ~(0xFU << (6U * 4U));
    GPIOA->AFRL |=  (14U  << (6U * 4U));
}


// ---------------------------------------------------------
// Configure TIM16 for PWM generation
// ------------------------------------------------------

static void tim16_init(void)
{
    // Turn on TIM16 peripheral clock
    RCC->APB2ENR |= (1U << 17);

    // Stop counter while configuring
    TIM16->CR1 &= ~(1U << 0);

    // PWM timer prescaler
    TIM16->PSC = PWM_PSC;

    // Enable ARR preload
    TIM16->CR1 |= (1U << 7);

    // OC1M = 110 -> PWM Mode 1
    TIM16->CCMR1 &= ~(7U << 4);
    TIM16->CCMR1 |=  (6U << 4);

    // Enable CCR1 preload
    TIM16->CCMR1 |= (1U << 3);

    // Active-high output
    TIM16->CCER &= ~(1U << 1);

    // Leave CC1E = 0 for now
    // set_freq() will enable output
    TIM16->CCER &= ~(1U << 0);

    // Main output enable
    TIM16->BDTR |= (1U << 15);

    // Load prescaler
    TIM16->EGR |= (1U << 0);

    // Start timer counter
    TIM16->CR1 |= (1U << 0);
}



// ------------------------------------------------------
// Configure TIM15 as note-duration timer
// ---------------------------------------------------------

static void tim15_init(void)
{
    // Enable TIM15 clock in RCC APB2ENR
    RCC->APB2ENR |= (1U << 16);

    // Stop while configuring
    TIM15->CR1 &= ~(1U << 0);

    // 80 MHz / 8000 = 10 kHz
    TIM15->PSC = DELAY_PSC;

    // Generate update event so PSC takes effect
    TIM15->EGR |= (1U << 0);

    // Clear UIF
    TIM15->SR &= ~(1U << 0);
}


// =========================================================
// PUBLIC FUNCTIONS
// =========================================================


void timers_init(void)
{
    clock_init();

    pwm_gpio_init();

    tim15_init();

    tim16_init();
}


// -----------------------------------------------------
// Change TIM16 PWM frequency
// ----------------------------------------------------

void set_freq(uint32_t freq_hz)
{
    // Disable output while changing frequency
    TIM16->CCER &= ~(1U << 0);

    if (freq_hz == 0U)
    {
        // Rest: simply leave output disabled
        return;
    }

    uint32_t timer_counter_hz =
        SYSTEM_CLOCK_HZ / (PWM_PSC + 1U);

    uint32_t arr =
        (timer_counter_hz / freq_hz) - 1U;

    TIM16->CNT  = 0U;
    TIM16->ARR  = arr;
    TIM16->CCR1 = (arr + 1U) / 2U;

    // Force update of preload registers
    TIM16->EGR |= (1U << 0);

    // Now expose PWM on channel 1
    TIM16->CCER |= (1U << 0);
}



// ---------------------------------------------------------
// Delay for specified number of milliseconds using TIM15
// ---------------------------------------------------------

void delay_ms(uint32_t duration_ms)
{
    if (duration_ms == 0U)
    {
        return;
    }

    // At 10 kHz:
    //
    // 1 ms = 10 counts
    uint32_t counts =
        duration_ms * COUNTS_PER_MS;

    // Stop TIM15 before setting up new delay
    TIM15->CR1 &= ~(1U << 0);

    // Start from zero
    TIM15->CNT = 0U;

    // Set terminal count
    TIM15->ARR = counts - 1U;

    // Generate update event so ARR is loaded
    TIM15->EGR |= (1U << 0);

    // EGR update sets UIF, so clear it before timing
    TIM15->SR &= ~(1U << 0);

    // Start timer
    TIM15->CR1 |= (1U << 0);

    // Wait until TIM15 reaches ARR and hardware sets UIF
    //
    // While the CPU waits here, TIM16 continues generating
    // the audio PWM completely independently
    while (!(TIM15->SR & (1U << 0)))
    {
    }

    // Stop timer
    TIM15->CR1 &= ~(1U << 0);

    // Clear flag for next note
    TIM15->SR &= ~(1U << 0);

  }

