// encoder.h
// Quadrature encoder driver: EXTI interrupts on both channels + a 1 MHz timestamp timer.

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "STM32L432KC.h"

// Encoder pins. Both are 5 V tolerant (FT) on the STM32L432KC
// (datasheet Table 14), and both land on the same EXTI9_5 interrupt line.
#define ENCODER_A      PA6
#define ENCODER_B      PA8
#define ENCODER_A_NUM  6
#define ENCODER_B_NUM  8

// 32-bit TIM2 runs free at 1 MHz -> 1 count = 1 us, wraps every ~71 min.
// Unsigned subtraction (later - earlier) handles the wrap.
#define TIMESTAMP_TIM  TIM2
#define TIMESTAMP_HZ   1000000u   // 1 count = 1 us

// A consistent copy of what the ISR has counted so far.
typedef struct {
    int32_t  position;   // signed edge count (+ = CW, - = CCW)
    uint32_t edgeTime;   // TIM2 time of the most recent edge (us)
    uint32_t badEdges;   // transitions where A and B both changed (missed edge)
} encoder_snapshot_t;

void     initEncoder(void);
uint32_t micros(void);
encoder_snapshot_t encoderSnapshot(void);

// Pure helpers (no hardware) so they can be checked on a laptop
int8_t   quadStep(uint8_t prevAB, uint8_t newAB);
int32_t  velocityMicroRevPerSec(int32_t dEdges, uint32_t dTimeUs, uint32_t edgesPerRev);

#endif // ENCODER_H
