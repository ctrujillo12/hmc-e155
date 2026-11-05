// encoder.c
// Quadrature encoder driver: EXTI on every edge of A and B, timestamped with TIM2
// Cheyenne Trujillo, 2026-11-04

#include "encoder.h"

// both A and B changed: missed edge
#define QUAD_ILLEGAL 2

// edges/us -> micro-rev/s
#define URPS_SCALE 1000000000000LL

// shared with the ISR
static volatile int32_t  position  = 0;
static volatile uint32_t edgeTime  = 0;
static volatile uint32_t badEdges  = 0;
static volatile uint8_t  lastAB    = 0;

// indexed by (prevAB << 2) | newAB, CW = 00 -> 10 -> 11 -> 01
static const int8_t QUAD_TABLE[16] = {
//  new: 00  01  10  11      prev
         0,           -1,           +1,            QUAD_ILLEGAL,   // 00
        +1,            0,            QUAD_ILLEGAL, -1,             // 01
        -1,            QUAD_ILLEGAL,  0,           +1,             // 10
         QUAD_ILLEGAL, +1,           -1,            0              // 11
};

int8_t quadStep(uint8_t prevAB, uint8_t newAB) {
    return QUAD_TABLE[((prevAB & 3) << 2) | (newAB & 3)];
}

// speed in micro-rev/s
int32_t velocityMicroRevPerSec(int32_t dEdges, uint32_t dTimeUs, uint32_t edgesPerRev) {
    if (dTimeUs == 0 || edgesPerRev == 0) return 0;
    int64_t num = (int64_t)dEdges * URPS_SCALE;
    int64_t den = (int64_t)dTimeUs * (int64_t)edgesPerRev;
    // round to nearest
    if (num >= 0) return (int32_t)((num + den / 2) / den);
    else          return (int32_t)((num - den / 2) / den);
}

static uint8_t readAB(void) {
    uint32_t idr = GPIOA->IDR;   // read both pins at once
    return (uint8_t)((((idr >> ENCODER_A_NUM) & 1) << 1) | ((idr >> ENCODER_B_NUM) & 1));
}

uint32_t micros(void) {
    return TIMESTAMP_TIM->CNT;
}

static void initTimestampTimer(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    TIMESTAMP_TIM->PSC = (SystemCoreClock / TIMESTAMP_HZ) - 1;  // 80 MHz / 80 = 1 MHz
    TIMESTAMP_TIM->ARR = 0xFFFFFFFF;                        // use all 32 bits
    TIMESTAMP_TIM->EGR = TIM_EGR_UG;                        // load the prescaler now
    TIMESTAMP_TIM->SR  = 0;
    TIMESTAMP_TIM->CNT = 0;
    TIMESTAMP_TIM->CR1 |= TIM_CR1_CEN;
}

void initEncoder(void) {
    gpioEnable(GPIO_PORT_A);
    pinMode(ENCODER_A, GPIO_INPUT);
    pinMode(ENCODER_B, GPIO_INPUT);

    // no pull resistors since the encoder outputs 5 V
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD8);

    initTimestampTimer();

    // route PA6 and PA8 to EXTI6 and EXTI8
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI6;
    SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI8;

    // trigger on both edges
    uint32_t mask = (1u << ENCODER_A_NUM) | (1u << ENCODER_B_NUM);
    EXTI->RTSR1 |= mask;
    EXTI->FTSR1 |= mask;
    EXTI->PR1    = mask;      // clear pending
    EXTI->IMR1  |= mask;

    lastAB = readAB();

    // EXTI lines 5-9 share one interrupt
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    __enable_irq();
}

// copy the ISR's variables with interrupts off
encoder_snapshot_t encoderSnapshot(void) {
    encoder_snapshot_t s;
    __disable_irq();
    s.position = position;
    s.edgeTime = edgeTime;
    s.badEdges = badEdges;
    __enable_irq();
    return s;
}

// runs on every edge of A or B
void EXTI9_5_IRQHandler(void) {
    uint32_t mask = (1u << ENCODER_A_NUM) | (1u << ENCODER_B_NUM);
    uint32_t pending = EXTI->PR1 & mask;
    if (!pending) return;

    // clear before reading the pins so a new edge re-triggers the interrupt
    EXTI->PR1 = pending;

    uint32_t now = TIMESTAMP_TIM->CNT;
    uint8_t  ab  = readAB();
    int8_t   step = quadStep(lastAB, ab);

    if (step == QUAD_ILLEGAL) {
        badEdges++;
    } else if (step != 0) {
        position += step;
        edgeTime  = now;
    }
    lastAB = ab;
}
