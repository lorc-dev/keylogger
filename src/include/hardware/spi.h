//
// Created by Lorenz on 3/11/2021.
//

#ifndef KEYLOGGER_SPI_H
#define KEYLOGGER_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "addressmap.h"

// See PrimeCell Synchronous Serial Port (PL022) Technical Reference Manual
// https://developer.arm.com/documentation/ddi0194/latest


// SPI registers (See Table 506)
typedef struct {
    uint32_t volatile sspcr0;   // Control register 0
    uint32_t volatile sspcr1;   // Control register 1
    uint32_t volatile sspdr;    // Data register
    uint32_t volatile sspsr;    // Status register
    uint32_t volatile sspcpsr;  // Clock prescale register
    uint32_t volatile sspimsc;  // Interupt mask set or clear register
    uint32_t volatile sspris;   // Raw interrupt status register
    uint32_t volatile sspmis;   // Masked interrupt register
    uint32_t volatile sspicr;   // Interrupt clear register
    uint32_t volatile sspdmacr; // DMA control register
} spi_hw_t;

#define spi0_hw ((spi_hw_t *const)SPI0_BASE)
#define spi1_hw ((spi_hw_t *const)SPI1_BASE)

// SSPCR0 register (See Table 507)
#define SPI_SSPCR0_SCR_BITS (0xfful<<8)   // Serial clock rate
#define SPI_SSPCR0_SPH_BIT  (0x01ul<<7)   // SSPCLKOUT phase
#define SPI_SSPCR0_SPO_BIT  (0x01ul<<6)   // SSPCLKOUT polarity
#define SPI_SSPCR0_FRF_BITS (0x03ul<<4)   // Frame format
#define SPI_SSPCR0_DSS_BITS (0x0ful)      // Data size select
#define SPI_SSPCR0_SCR_LSB  8ul
#define SPI_SSPCR0_SPH_LSB  7ul
#define SPI_SSPCR0_SPO_LSB  6ul
#define SPI_SSPCR0_FRF_LSB  4ul
#define SPI_SSPCR0_DSS_LSB  0ul

// SSPCR1 register (See Table 508)
#define SPI_SSPCR1_SOD_BIT  (1ul<<3)    // Slave mode output disable
#define SPI_SSPCR1_MS_BIT   (1ul<<2)    // Master or slave mode select (0:Master, 1:Slave)
#define SPI_SSPCR1_SSE_BIT  (1ul<<1)    // Synchronous serial port enable
#define SPI_SSPCR1_LBM_BIT  (1ul)       // Loop back mode
#define SPI_SSPCR1_SOD_LSB  3ul
#define SPI_SSPCR1_MS_LSB   2ul
#define SPI_SSPCR1_SSE_LSB  1ul
#define SPI_SSPCR1_LBM_LSB  0ul

// SSPSR register (See Table 510)
#define SPI_SSPSR_BSY_BIT   (1ul<<4)    // Busy flag
#define SPI_SSPSR_RFF_BIT   (1ul<<3)    // Receive FIFO full
#define SPI_SSPSR_RNE_BIT   (1ul<<2)    // Receive FIFO not empty
#define SPI_SSPSR_TNF_BIT   (1ul<<1)    // Transmit FIFO not full
#define SPI_SSPSR_TFE_BIT   (1ul)       // Transmit FIFO empty




// Function prototypes
void spi_init(spi_hw_t *spi, uint32_t clk_rate);
static inline void spi_set_clk_rate(spi_hw_t *spi, uint32_t clk_rate);
static inline void spi_set_format(spi_hw_t *spi, uint8_t data_bits, bool clk_polarity, bool clk_phase);
void spi_set_slave(spi_hw_t *spi, bool slave);
static inline bool spi_is_writable(spi_hw_t *spi);
static inline bool spi_is_readable(spi_hw_t *spi);
static inline bool spi_is_busy(spi_hw_t *spi);
void spi_write(spi_hw_t *spi, uint8_t *src, uint32_t len);
void spi_read(spi_hw_t *spi, uint8_t repeat_tx_data, uint8_t *dst, uint32_t len);
void spi_write_read(spi_hw_t *spi, uint8_t *src, uint8_t *dst, uint32_t len);

#endif //KEYLOGGER_SPI_H
