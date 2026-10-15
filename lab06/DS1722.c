// DS1722.c
// DS1722 digital thermometer driver over SPI (CE active high on PA8)

#include "DS1722.h"

#define DS1722_ADDR_CFG_READ   0x00
#define DS1722_ADDR_CFG_WRITE  0x80
#define DS1722_ADDR_TEMP_LSB   0x01   // MSB at 0x02; burst read auto-advances 01h -> 02h

static void ceHigh(void) { digitalWrite(DS1722_CE_PIN, 1); }
static void ceLow(void)  { digitalWrite(DS1722_CE_PIN, 0); }

// Max conversion time in ms for a given config byte (datasheet Table 5)
static uint32_t conversionMs(uint8_t cfg) {
    if (cfg & 0x08) return 1200;                 // R2=1 -> 12-bit
    switch ((cfg >> 1) & 0x3) {                  // R1:R0
        case 0:  return 75;                      // 8-bit
        case 1:  return 150;                     // 9-bit
        case 2:  return 300;                     // 10-bit
        default: return 600;                     // 11-bit
    }
}

static void writeConfig(uint8_t cfg) {
    ceHigh();
    spiSendReceive(DS1722_ADDR_CFG_WRITE);
    spiSendReceive(cfg);
    ceLow();
}

void initTempSensor(void) {
    ceLow();
    writeConfig(DS1722_CFG_12BIT);               // powers up in shutdown; this clears SD
    delay_millis(TIM15, 1200);                   // let the first 12-bit conversion finish
}

void writeToTempSensor(uint8_t cfg) {
    // Accept only 111x xxx0 patterns with 1SHOT clear: 0xE0, E2, E4, E6, E8
    if ((cfg & 0xF1) != 0xE0 || ((cfg >> 1) & 0x7) > 4) return;

    writeConfig(cfg);
    delay_millis(TIM15, conversionMs(cfg));      // CE is already low while we wait
}

uint8_t readConfigFromTempSensor(void) {
    ceHigh();
    spiSendReceive(DS1722_ADDR_CFG_READ);
    uint8_t cfg = spiSendReceive(0x00);
    ceLow();
    return cfg;
}

double readFromTempSensor(void) {
    // One burst transaction: address 0x01, then LSB, then MSB (address auto-increments)
    ceHigh();
    spiSendReceive(DS1722_ADDR_TEMP_LSB);
    uint8_t lsb = spiSendReceive(0x00);
    uint8_t msb = spiSendReceive(0x00);
    ceLow();
    return calc_temp(msb, lsb);
}

double calc_temp(uint8_t hi, uint8_t lo) {
    // {hi, lo} is a 16-bit two's complement value with 8 fractional bits.
    // The signed cast handles negative temperatures.
    int16_t raw = (int16_t)(((uint16_t)hi << 8) | lo);
    return raw / 256.0;
}



