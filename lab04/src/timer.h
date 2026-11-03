
// config file for timer.h

// function defs

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timers_init(void);
void set_freq(uint32_t freq_hz);
void delay_ms(uint32_t duration_ms);

#endif

