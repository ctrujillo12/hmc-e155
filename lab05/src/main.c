// main.c
// Lab 5: measures motor speed and direction with a quadrature encoder, prints once a second
// Cheyenne Trujillo, 2026-11-04

#include "main.h"
#include <stdio.h>

#define URPS_PER_LSB   100     // print to 0.0001 rev/s
#define DECIMAL_SCALE  10000

// prints micro-rev/s as e.g. "+2.4571"
static void printSpeed(int32_t urps) {
    char sign = (urps < 0) ? '-' : '+';
    uint32_t mag = (urps < 0) ? (uint32_t)(-urps) : (uint32_t)urps;
    mag = (mag + URPS_PER_LSB / 2) / URPS_PER_LSB;
    const char *dir = (urps > 0) ? "CW" : (urps < 0) ? "CCW" : "stopped";
    printf("speed: %c%lu.%04lu rev/s  (%s)\n",
           sign, (unsigned long)(mag / DECIMAL_SCALE), (unsigned long)(mag % DECIMAL_SCALE), dir);
}

int main(void) {
    configureFlash();
    configureClock();   // 80 MHz from the PLL
    initEncoder();

    encoder_snapshot_t ref = encoderSnapshot();
    uint32_t lastReport    = micros();
    uint32_t lastSnapTime  = lastReport;
    uint32_t lastBadEdges  = 0;
    int32_t  speed_urps    = 0;                  // micro-rev/s
    int      stopped       = 1;

    printf("Lab 5 encoder: %d edges per output rev (gear %d:1)\n", EDGES_PER_REV, GEAR_RATIO);

    while (1) {
        uint32_t now = micros();
        if ((uint32_t)(now - lastReport) < REPORT_PERIOD_US) continue;
        lastReport += REPORT_PERIOD_US;

        encoder_snapshot_t s = encoderSnapshot();
        uint32_t prevSnapTime = lastSnapTime;
        lastSnapTime = now;
        int32_t  dEdges = s.position - ref.position;

        if (dEdges != 0) {
            if (stopped) {
                // just started moving: measure from the last snapshot
                speed_urps = velocityMicroRevPerSec(dEdges, s.edgeTime - prevSnapTime, EDGES_PER_REV);
                stopped = 0;
            } else {
                speed_urps = velocityMicroRevPerSec(dEdges, s.edgeTime - ref.edgeTime, EDGES_PER_REV);
            }
            ref = s;
        } else if ((uint32_t)(now - ref.edgeTime) > STOP_TIMEOUT_US) {
            speed_urps = 0;
            stopped = 1;
            ref = s;
        }

        printSpeed(speed_urps);

        if (s.badEdges != lastBadEdges) {
            printf("  warning: %lu missed edges so far\n", (unsigned long)s.badEdges);
            lastBadEdges = s.badEdges;
        }
    }
}
