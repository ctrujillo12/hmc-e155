// DS1722.h
// DS1722 digital thermometer driver (SPI mode, CE active high on PA8)

#ifndef DS1722_H
#define DS1722_H

#include <stdint.h>
#include "STM32L432KC.h"

#define DS1722_CE_PIN PA8

// Config bytes: 111 1SHOT R2 R1 R0 SD, with SD=0 (continuous), 1SHOT=0
#define DS1722_CFG_8BIT   0xE0
#define DS1722_CFG_9BIT   0xE2
#define DS1722_CFG_10BIT  0xE4
#define DS1722_CFG_11BIT  0xE6
#define DS1722_CFG_12BIT  0xE8

// Sets continuous conversion at 12-bit and waits for the first conversion.
void initTempSensor(void);

// Writes a config byte (0xE0..0xE8) and waits one full conversion time.
// Ignores bytes that aren't a valid resolution config (e.g. 0).
void writeToTempSensor(uint8_t cfg);

// Reads the config register (for checking the SPI link; expect what you wrote).
uint8_t readConfigFromTempSensor(void);

// Reads temperature registers and returns degrees C.
double readFromTempSensor(void);

// Converts raw {MSB, LSB} to degrees C (16-bit two's complement, /256).
double calc_temp(uint8_t hi, uint8_t lo);

#endif


