//
// Created by Lorenz on 3/11/2021.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/hardware/spi.h"
#include "../include/hardware/resets.h"
#include "../include/hardware/clocks.h"
#include "../include/lib/math.h"

/**
 * Initializes and configures the selected SPI controller.
 * By default it will be in master mode
 *
 * @param spi
 * @param clk_rate
 */
void spi_init(spi_hw_t *spi, uint32_t clk_rate) {
    // Bring the spi controller in a know state
    uint32_t spi_select_reset_mask = spi == spi0_hw ? RESETS_RESET_SPI0 : RESETS_RESET_SPI1;
    reset_subsystem(spi_select_reset_mask);
    unreset_subsystem_wait(spi_select_reset_mask);

    // Set the baud rate and format
    spi_set_clk_rate(spi, clk_rate);
    spi_set_format(spi, 8, 0, 0);

    // Enable
    spi->sspcr1 |= SPI_SSPCR1_SSE_BIT;
}

/**
 * Calculates and sets the correct divider values for the given clk rate
 * The set clk rate is less or equal to the given clk rate
 *
 * @param spi
 * @param clk_rate
 */
static inline void spi_set_clk_rate(spi_hw_t *spi, uint32_t clk_rate) {
    uint32_t freq_in = clock_get_hz(clk_peri);
    uint32_t prescale = 127;
    uint32_t postdivide = 255;

    // Find optimal values for the prescaler and the post divider
    // Prescale is an even number from 2 to 254 inclusive
    // Post-divide is an number from 1 to 256 inc inclusive
    uint32_t div = ceil_division(freq_in, clk_rate);
    for (uint16_t ps = 2; ps <= 254; ps += 2) {
        if (ps * 255 < div)
            continue;
        uint32_t pd = ceil_division(div, ps);
        if (1 <= pd && pd <= 256 && ps * pd < prescale * postdivide) {
            prescale = ps;
            postdivide = pd;
        }
    }

    // Write the values to the correct registers
    // Postdivide value = (1+SCR) => postdivide - 1 [0-255]
    spi->sspcpsr = prescale;
    spi->sspcr0 = (spi->sspcr0 & ~SPI_SSPCR0_SCR_BITS) | (((postdivide - 1) << SPI_SSPCR0_SCR_LSB) & SPI_SSPCR0_SCR_BITS);
}

/**
 * Configures how the data is serialised and de-serialised
 *
 * @param spi
 * @param data_bits Number of data bits [4-16]
 * @param clk_polarity
 * @param clk_phase
 */
static inline void spi_set_format(spi_hw_t *spi, uint8_t data_bits, bool clk_polarity, bool clk_phase) {
    // DSS values 0-2 are reserved. 3 = 4bit, 4 = 5bit, ...
    uint32_t value = (uint32_t)(data_bits - 1) << SPI_SSPCR0_DSS_LSB |
                     (uint32_t)clk_polarity << SPI_SSPCR0_SPO_LSB |
                     (uint32_t)clk_phase << SPI_SSPCR0_SPH_LSB;

    uint32_t mask = SPI_SSPCR0_DSS_BITS | SPI_SSPCR0_SPO_BIT | SPI_SSPCR0_SPH_BIT;

    spi->sspcr0 = (spi->sspcr0 & ~mask) | (value & mask);
}

/**
 * Configures the SPI controller for master or slave.
 *
 * @param spi
 * @param slave 0 = master, 1 = slave
 */
void spi_set_slave(spi_hw_t *spi, bool slave) {
    if (slave)
        spi->sspcr1 |= SPI_SSPCR1_MS_BIT;
    else
        spi->sspcr1 &= ~SPI_SSPCR1_MS_BIT;
}

/**
 * Checks if the transmit FIFO buffer is not full
 *
 * @param spi
 * @return
 */
static inline bool spi_is_writable(spi_hw_t *spi) {
    return (spi->sspsr & SPI_SSPSR_TNF_BIT);
}

/**
 * Checks if there is data in the receive FIFO
 *
 * @param spi
 * @return
 */
static inline bool spi_is_readable(spi_hw_t *spi) {
    return (spi->sspsr & SPI_SSPSR_RNE_BIT);
}