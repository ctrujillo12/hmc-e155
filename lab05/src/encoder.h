// encoder.h
// Quadrature encoder driver
// Cheyenne Trujillo, 2026-11-04

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "STM32L432KC.h"

// encoder pins (5 V tolerant)
#define ENCODER_A      PA6
#define ENCODER_B      PA8
#define ENCODER_A_NUM  6
#define ENCODER_B_NUM  8

// 1 MHz timer for edge timestamps
#define TIMESTAMP_TIM  TIM2
#define TIMESTAMP_HZ   1000000u

typedef struct {
    int32_t  position;   // signed edge count (+ = CW, - = CCW)
    uint32_t edgeTime;   // TIM2 time of the most recent edge (us)
} encoder_snapshot_t;

void     initEncoder(void);

// counts microseconds since the timer started 
uint32_t micros(void); 

// makes sure interrupts don't change the values while copying
encoder_snapshot_t encoderSnapshot(void); 

int32_t  velocityMicroRevPerSec(int32_t dEdges, uint32_t dTimeUs, uint32_t edgesPerRev); // calc speed in micro-rev/s 

#endif // ENCODER_H
