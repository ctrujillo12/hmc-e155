// main.h
// Lab 5: quadrature encoder motor speed
// Cheyenne Trujillo, 2026-11-04

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include "encoder.h"

// 25GA-370 motor, 34:1 gearbox
#define GEAR_RATIO       34
#define PULSES_PER_MOTOR_REV 12   // per channel, motor shaft

// count every edge of A and B 
#define EDGES_PER_REV    (4 * PULSES_PER_MOTOR_REV * GEAR_RATIO)

// print once a second
#define REPORT_PERIOD_US 1000000u

// no edge for this long = stopped
#define STOP_TIMEOUT_US  3000000u

#endif // MAIN_H
