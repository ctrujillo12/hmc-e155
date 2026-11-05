// main.c
// Lab 5: measures motor speed and direction with a quadrature encoder, prints once a second
// Cheyenne Trujillo, 2026-11-04

#include "main.h"
#include <stdio.h>

#define URPS_PER_LSB   100     // print to 0.0001 rev/s
#define DECIMAL_SCALE  10000


// (no float printf, so it splits the number into whole part and 4 decimals by hand)
static void printSpeed(int32_t urps) {
    // sign and size separately (+/- then the positive number)
    char sign = (urps < 0) ? '-' : '+';
    uint32_t mag = (urps < 0) ? (uint32_t)(-urps) : (uint32_t)urps;

    // micro-rev/s -> units of 0.0001 rev/s, rounded
    mag = (mag + URPS_PER_LSB / 2) / URPS_PER_LSB;

    // positive = CW, negative = CCW, 0 = stopped
    const char *dir = (urps > 0) ? "CW" : (urps < 0) ? "CCW" : "stopped";

    // whole part . 4 decimal digits
    // (this printf doesn't support %lu or %04, so zero-pad the decimals by hand)
    unsigned int whole = (unsigned int)(mag / DECIMAL_SCALE);
    unsigned int frac  = (unsigned int)(mag % DECIMAL_SCALE);
    char fracStr[5];
    for (int i = 3; i >= 0; i--) {
        fracStr[i] = (char)('0' + frac % 10);
        frac /= 10;
    }
    fracStr[4] = '\0';

    printf("speed: %c%u.%s rev/s  (%s)\n", sign, whole, fracStr, dir);
}
// prints micro-rev/s as "+2.4571"

int main(void) {
    configureFlash();
    configureClock();   // 80 MHz from the PLL
    initEncoder();      // pins, interrupts and the 1 MHz timer (TIM2)

    // ref = the encoder reading from the last time we calculated a speed
    // each second we compare the new reading to ref to see how far it moved
    encoder_snapshot_t ref = encoderSnapshot();
    uint32_t lastReport    = micros();          // when we last printed
    uint32_t lastSnapTime  = lastReport;        // when we last took a snapshot
    int32_t  speed_urps    = 0;                 // speed in micro-rev/s (+ = CW, - = CCW)
    int      stopped       = 1;                 // start out assuming the motor isn't moving

    printf("%d edges per output rev (gear %d:1)\n", EDGES_PER_REV, GEAR_RATIO);

    while (1) {
        // wait until 1 s has passed 
        uint32_t now = micros();
        if ((uint32_t)(now - lastReport) < REPORT_PERIOD_US) continue;
        lastReport += REPORT_PERIOD_US;   // += (not = now) so it doesn't slowly drift

        // grab the latest encoder data 
        encoder_snapshot_t s = encoderSnapshot();
        uint32_t prevSnapTime = lastSnapTime;
        lastSnapTime = now;
        int32_t  dEdges = s.position - ref.position; // how many steps since last time

        // calculate the speed 
        if (dEdges != 0) {
            if (stopped) {
                // just started moving
                speed_urps = velocityMicroRevPerSec(dEdges, s.edgeTime - prevSnapTime, EDGES_PER_REV);
                stopped = 0;
            } else {
                // normal case
                speed_urps = velocityMicroRevPerSec(dEdges, s.edgeTime - ref.edgeTime, EDGES_PER_REV);
            }
            ref = s;   // this reading becomes the starting point for next time
        } else if ((uint32_t)(now - ref.edgeTime) > STOP_TIMEOUT_US) {
            // no edges for over 3 s: call it stopped
            speed_urps = 0;
            stopped = 1;
            ref = s;
        }
        // (no edges but less than 3 s: just keep showing the last speed)
        printSpeed(speed_urps);
    }
}
