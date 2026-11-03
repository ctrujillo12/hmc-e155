// main.h
// Lab 5: Interrupts — quadrature encoder motor speed
// Cheyenne Trujillo

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include "encoder.h"

// ---------------------------------------------------------------
// Motor settings: 25GA-370 with a 34:1 gearbox
// ---------------------------------------------------------------
#define GEAR_RATIO       34
#define PULSES_PER_MOTOR_REV 12   // per channel, on the motor shaft (from lab page)

// We count every rising AND falling edge of both A and B ("x4 decoding"),
// so each output-shaft revolution is 4 * 12 * gear ratio edges.
#define EDGES_PER_REV    (4 * PULSES_PER_MOTOR_REV * GEAR_RATIO)

// How often the speed is printed (spec: at least 1 Hz)
#define REPORT_PERIOD_US 1000000u

// If no edge shows up for this long, call the motor stopped.
// Slowest speed we can report = 1 edge / STOP_TIMEOUT_US.
#define STOP_TIMEOUT_US  3000000u

#endif // MAIN_H
