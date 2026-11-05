// encoder.c
// Quadrature encoder driver for Lab 5.
//
// How it works:
//   - PA6 (A) and PA8 (B) fire an EXTI interrupt on every rising and falling edge.
//   - The ISR reads A and B together, and a lookup table turns (old AB, new AB)
//     into +1, -1 or 0. Adding that to `position` gives a signed count, so
//     direction falls out of the sign; no separate direction flag needed.
//   - The ISR also stamps the time of the edge from a 1 MHz free-running TIM2.
//     main() divides edges by the time between the first and last edge it saw,
//     which keeps the speed accurate at very low speeds (see report).

#include "encoder.h"

// QUAD_TABLE value for an illegal transition (A and B both changed -> missed edge)
#define QUAD_ILLEGAL 2

// edges/us -> micro-rev/s needs a factor of (1e6 us/s) * (1e6 micro-rev/rev)
#define URPS_SCALE 1000000000000LL

// Shared with the ISR -> volatile
static volatile int32_t  position  = 0;
static volatile uint32_t edgeTime  = 0;
static volatile uint32_t badEdges  = 0;
static volatile uint8_t  lastAB    = 0;

// Quadrature lookup table, indexed by (prevAB << 2) | newAB.
// Forward (CW) sequence is AB = 00 -> 10 -> 11 -> 01 -> 00 (A leads B).
//   +1 = one step CW, -1 = one step CCW, 0 = no change,
//    QUAD_ILLEGAL = both bits flipped: we missed an edge
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

// Speed in millionths of a rev/s (integer, so printf doesn't need float support).
// 64-bit math: dEdges * 1e12 is ~1e17 at full speed, still fits in int64.
int32_t velocityMicroRevPerSec(int32_t dEdges, uint32_t dTimeUs, uint32_t edgesPerRev) {
    if (dTimeUs == 0 || edgesPerRev == 0) return 0;
    int64_t num = (int64_t)dEdges * URPS_SCALE;
    int64_t den = (int64_t)dTimeUs * (int64_t)edgesPerRev;
    // round to nearest instead of truncating
    if (num >= 0) return (int32_t)((num + den / 2) / den);
    else          return (int32_t)((num - den / 2) / den);
}

static uint8_t readAB(void) {
    uint32_t idr = GPIOA->IDR;   // read both pins in ONE register read
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
    // GPIO: A and B are plain inputs.
    gpioEnable(GPIO_PORT_A);
    pinMode(ENCODER_A, GPIO_INPUT);
    pinMode(ENCODER_B, GPIO_INPUT);

    // No internal pull-up/pull-down: the encoder drives 5 V, and the datasheet
    // says a FT pin can only take > VDD + 0.3 V with its pull resistors OFF.
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD8);

    initTimestampTimer();

    // SYSCFG: route port A to EXTI6 and EXTI8 (0b000 = port A)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI6;   // EXTICR2 holds EXTI4..7
    SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI8;   // EXTICR3 holds EXTI8..11

    // EXTI: unmask, and trigger on both edges
    uint32_t mask = (1u << ENCODER_A_NUM) | (1u << ENCODER_B_NUM);
    EXTI->RTSR1 |= mask;
    EXTI->FTSR1 |= mask;
    EXTI->PR1    = mask;      // clear anything stale (write-1-to-clear)
    EXTI->IMR1  |= mask;

    lastAB = readAB();        // start the state machine from where the shaft is now

    // NVIC: EXTI lines 5..9 share one interrupt
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    __enable_irq();
}

// Copy the ISR's variables with interrupts off for a moment, so position and
// edgeTime always belong to the same edge.
encoder_snapshot_t encoderSnapshot(void) {
    encoder_snapshot_t s;
    __disable_irq();
    s.position = position;
    s.edgeTime = edgeTime;
    s.badEdges = badEdges;
    __enable_irq();
    return s;
}

// Runs on every edge of A or B.
void EXTI9_5_IRQHandler(void) {
    uint32_t mask = (1u << ENCODER_A_NUM) | (1u << ENCODER_B_NUM);
    uint32_t pending = EXTI->PR1 & mask;
    if (!pending) return;

    // Clear FIRST, then read the pins. If another edge lands after this read,
    // it sets the flag again and we come right back in, so nothing is lost.
    // Note: plain "=" not "|=". PR1 is write-1-to-clear, so "|=" would also
    // clear every other pending EXTI line.
    EXTI->PR1 = pending;

    uint32_t now = TIMESTAMP_TIM->CNT;
    uint8_t  ab  = readAB();
    int8_t   step = quadStep(lastAB, ab);

    if (step == QUAD_ILLEGAL) {
        badEdges++;            // both changed: an edge was missed, direction unknown
    } else if (step != 0) {
        position += step;
        edgeTime  = now;
    }
    lastAB = ab;
}
