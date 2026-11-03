// main.c
// Lab 5: Interrupts — measure a DC motor's speed and direction with a quadrature encoder.
// Cheyenne Trujillo
//
// Every REPORT_PERIOD_US (1 s) this prints something like
//     speed: +2.4571 rev/s  (CW)
// to the SEGGER debug terminal through printf (course printf tutorial).
//
// Speed = (edges counted) / (time between the first and last of those edges).
// Using the edge timestamps instead of "edges in the last 1 s" removes the
// +/- 1 edge rounding error, which matters a lot at low speeds.

#include "main.h"
#include <stdio.h>

// Print an integer number of micro-rev/s as e.g. "+2.4571" (4 decimals) without float printf.
static void printSpeed(int32_t urps) {
    char sign = (urps < 0) ? '-' : '+';
    uint32_t mag = (urps < 0) ? (uint32_t)(-urps) : (uint32_t)urps;
    mag = (mag + 50) / 100;                       // round to 0.0001 rev/s
    const char *dir = (urps > 0) ? "CW" : (urps < 0) ? "CCW" : "stopped";
    printf("speed: %c%lu.%04lu rev/s  (%s)\n",
           sign, (unsigned long)(mag / 10000), (unsigned long)(mag % 10000), dir);
}

int main(void) {
    configureFlash();
    configureClock();   // 80 MHz from the PLL
    initEncoder();

    encoder_snapshot_t ref = encoderSnapshot();  // last edge used for the previous speed
    uint32_t lastReport    = micros();
    uint32_t lastSnapTime  = lastReport;         // when the previous snapshot was taken
    uint32_t lastBadEdges  = 0;
    int32_t  speed_urps    = 0;                  // micro-rev/s, signed
    int      stopped       = 1;

    printf("Lab 5 encoder: %d edges per output rev (gear %d:1)\n", EDGES_PER_REV, GEAR_RATIO);

    while (1) {
        uint32_t now = micros();
        if ((uint32_t)(now - lastReport) < REPORT_PERIOD_US) continue;
        lastReport += REPORT_PERIOD_US;   // fixed 1 Hz cadence, no drift

        encoder_snapshot_t s = encoderSnapshot();
        uint32_t prevSnapTime = lastSnapTime;
        lastSnapTime = now;
        int32_t  dEdges = s.position - ref.position;

        if (dEdges != 0) {
            if (stopped) {
                // Just started moving. The old reference edge is from before the
                // stop, so measure from the previous snapshot instead.
                speed_urps = velocityMicroRevPerSec(dEdges, s.edgeTime - prevSnapTime, EDGES_PER_REV);
                stopped = 0;
            } else {
                speed_urps = velocityMicroRevPerSec(dEdges, s.edgeTime - ref.edgeTime, EDGES_PER_REV);
            }
            ref = s;
        } else if ((uint32_t)(now - ref.edgeTime) > STOP_TIMEOUT_US) {
            // No edge in a long time -> stopped
            speed_urps = 0;
            stopped = 1;
            ref = s;
        }
        // else: very slow, no edge this second -> keep showing the last speed

        printSpeed(speed_urps);

        if (s.badEdges != lastBadEdges) {
            printf("  warning: %lu missed edges so far\n", (unsigned long)s.badEdges);
            lastBadEdges = s.badEdges;
        }
    }
}
