// STM32L432KC_SPI.c
// SPI1 master driver for STM32L432KC (matches the course-provided STM32L432KC_SPI.h)

#include "STM32L432KC_SPI.h"

void initSPI(int br, int cpol, int cpha) {
    // Make sure the SPI1 peripheral clock is on (APB2ENR.SPI1EN)
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Disable SPI and start from a clean slate so re-init is safe
    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    // CR2: 8-bit data size, RXNE fires at 8 bits, Motorola frame format (FRF=0),
    // no hardware NSS output (SSOE=0), no NSS pulse (NSSP=0)
    SPI1->CR2 = _VAL2FLD(SPI_CR2_DS, 0b0111) | SPI_CR2_FRXTH;

    // CR1: baud rate, polarity, phase
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br & 0x7);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol & 1);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha & 1);

    // MSB first, full duplex, 2-line (LSBFIRST, RXONLY, BIDIMODE already 0 from reset above)

    // Software slave management: internal NSS held high so master mode isn't faulted
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

    // Master mode (after SSI is set)
    SPI1->CR1 |= SPI_CR1_MSTR;

    // Enable last
    SPI1->CR1 |= SPI_CR1_SPE;
}

char spiSendReceive(char send) {
    while (!(SPI1->SR & SPI_SR_TXE));                       // wait until TX buffer empty
    *(volatile uint8_t *)&SPI1->DR = (uint8_t)send;         // 8-bit write: one byte into the FIFO
    while (!(SPI1->SR & SPI_SR_RXNE));                      // wait for received byte
    uint8_t rx = *(volatile uint8_t *)&SPI1->DR;            // 8-bit read
    while (SPI1->SR & SPI_SR_BSY);                          // bus idle before caller drops CE
    return (char)rx;
}


