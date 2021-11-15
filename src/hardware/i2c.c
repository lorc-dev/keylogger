//
// Created by Lorenz on 7/11/2021.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/hardware/i2c.h"
#include "../include/hardware/resets.h"
#include "../include/hardware/clocks.h"
#include "../include/lib/math.h"


/**
 * Initializes and configures an I2C controller.
 * @note By default, the controller is set in Master mode. Use i2c_set_slave_mode() to change this
 * @note Sets Fast mode and 7-bit addressing
 *
 * @param i2c
 * @param clk_rate
 */
void i2c_init(i2c_hw_t *i2c, uint32_t clk_rate) {
    // Bring the I2C controller in a know state
    uint32_t i2c_select_reset_mask = i2c == i2c0_hw ? RESETS_RESET_I2C0 : RESETS_RESET_I2C1;
    reset_subsystem(i2c_select_reset_mask);
    unreset_subsystem_wait(i2c_select_reset_mask);

    // Make sure the controller is disabled
    i2c->ic_enable = 0;

    // Configure as fast-mode and 7-bit addressing
    i2c->ic_con = I2C_IC_CON_SPEED_VALUE_FM << I2C_IC_CON_SPEED_LSB |
                  I2C_IC_CON_MASTER_MODE_BIT |
                  I2C_IC_CON_IC_SLAVE_DISABLE_BIT |
                  I2C_IC_CON_IC_RESTART_EN_BIT |
                  I2C_IC_CON_TX_EMPTY_CTRL_BIT;

    // Make the depth of the rx and tx FIFO effectively 1
    // The range is 0-255 where 0 = 1 entry and 255 for 256 entries
    i2c->ic_tx_tl = 0;
    i2c->ic_rx_tl = 0;

    // Set clk rate
    i2c_set_clk_rate(i2c, clk_rate);

    // Enable the controller
    i2c->ic_enable = 1;
}

/**
 * Set the I2C bus frequency and related parameters.
 * @note The I2C controller must be disabled before running this function
 *
 * @param i2c
 * @param clk_rate
 */
static inline void i2c_set_clk_rate(i2c_hw_t *i2c, uint32_t clk_rate) {
    // The I2C controller runs from the sys clock
    uint32_t freq_in = clock_get_hz(clk_sys);

    uint32_t period = integer_division(freq_in, clk_rate);
    uint32_t low_count = (period + 1) >> 1;  // Divide by 2 and round up
    uint32_t high_count = period - low_count;

    // According to the I2C specification:
    // Data hold time:  min 300ns
    // Data setup time: min 100ns
    uint32_t sda_tx_hold_count = integer_division(freq_in * 3, 10000000);
    uint32_t sda_setup_count = integer_division(freq_in, 10000000);

    // Set the high and low count
    i2c->ic_fs_scl_lcnt = low_count;
    i2c->ic_fs_scl_hcnt = high_count;

    // Set sda_tx_hold (15:0) with the calculated value
    i2c->ic_sda_hold = (i2c->ic_sda_hold & ~I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS) |
                       (sda_tx_hold_count & I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS);

    // Set sda_setup. The length of the setup count is calculated using IC_SDA_SETUP - 1
    i2c->ic_sda_setup = sda_setup_count + 1;

    // Set the spike suppression limit
    // According to the I2C specification (Table 4):
    // min:     0 ns
    // max :    50 ns
    // The FS_SPKLEN registers stores the duration, measured in ic_clk_cycles
    // Default value is 0x07, with a 125MHz sys_clk > 50 ns
    // Set the minimum value 1
    i2c->ic_fs_spklen = 1;
}

/**
 * Set the I2C controller to master or slave mode.
 * @note When set to master mode, the address parameter is ignored
 * @note The I2C controller will be temporally disabled
 *
 * @param i2c
 * @param slave Slave: 1 | Master: 0
 * @param address 7-bit address (Slave mode only)
 */
void i2c_set_slave_mode(i2c_hw_t *i2c, bool slave, uint8_t address) {
    // The I2C controller must be disabled
    i2c->ic_enable = 0;

    if(slave) {
        i2c->ic_con &= ~(I2C_IC_CON_MASTER_MODE_BIT | I2C_IC_CON_IC_SLAVE_DISABLE_BIT);
        i2c->ic_sar = address;
    } else {
        i2c->ic_con |= (I2C_IC_CON_MASTER_MODE_BIT | I2C_IC_CON_IC_SLAVE_DISABLE_BIT);
    }

    i2c->ic_enable = 1;
}

/**
 * Sets the target address
 * @note The I2C controller will temporally be disabled
 *
 * @param i2c
 * @param address 7-bit address
 */
static inline void i2c_set_target_address(i2c_hw_t *i2c, uint8_t address) {
    // The I2C controller must be disabled to change the value of the tar register
    i2c->ic_enable = 0;
    i2c->ic_tar = (uint32_t)address;
    i2c->ic_enable = 1;
}

/**
 * Writes len bytes from src to address
 *
 * @param i2c
 * @param address 7-bit address
 * @param src
 * @param len
 * @param start 1: Start transaction | 0: don't start transaction
 * @param stop  1: Stop transaction | 0: don't stop transaction
 * @return 0: no errors occurred
 */
uint32_t i2c_write(i2c_hw_t *i2c, uint8_t address, uint8_t *src, uint32_t len, bool start, bool stop) {
    i2c_set_target_address(i2c, address);
    uint32_t abort_source = 0;

    for (uint32_t i = 0; i < len; i++) {
        uint32_t restart_value = (uint32_t)(start && i == 0);
        uint32_t stop_value = (uint32_t)(stop && i == len - 1);

        i2c->ic_data_cmd = restart_value << I2C_IC_DATA_CMD_RESTART_LSB |
                           stop_value << I2C_IC_DATA_CMD_STOP_LSB |
                           I2C_IC_DATA_CMD_CMD_VALUE_MASTER_WRITE << I2C_IC_DATA_CMD_CMD_LSB|
                           *src++;

        // Wait until the transmission is complete
        // TX_EMPTY_CTRL must be set (done in i2c_init())
        while (!(i2c->ic_raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_EMPTY_BIT));

        // Check if there was an abort
        abort_source = i2c->ic_tx_abrt_source;
        // Clear the abort flag and reason, clear on read
        i2c->ic_clr_tx_abrt;

        // If the transaction was completed or aborted, wait until the STOP flag is set
        if (stop_value || abort_source) {
            while (!(i2c->ic_raw_intr_stat & I2C_IC_RAW_INTR_STAT_STOP_DET_BIT));
        }

        // Stop sending data when aborted
        if (abort_source)
            break;

    }

    return abort_source;
}

/**
 * Reads len bytes from address into dst
 *
 * @param i2c
 * @param address 7-bit address
 * @param dst
 * @param len
 * @param start 1: Start transaction | 0: don't star transaction
 * @param stop  1: Stop transaction | 0: don't stop transaction
 * @return 0: no errors occurred
 */
uint32_t i2c_read(i2c_hw_t *i2c, uint8_t address, uint8_t *dst, uint32_t len, bool start, bool stop) {
    i2c_set_target_address(i2c, address);
    uint32_t abort_source;

    for (uint32_t i = 0; i < len; i++) {
        uint32_t restart_value = (uint32_t)(start && i == 0);
        uint32_t stop_value = (uint32_t)(stop && i == len - 1);

        i2c->ic_data_cmd = restart_value << I2C_IC_DATA_CMD_RESTART_LSB |
                           stop_value << I2C_IC_DATA_CMD_STOP_LSB |
                           I2C_IC_DATA_CMD_CMD_VALUE_MASTER_READ << I2C_IC_DATA_CMD_CMD_LSB;

        // Wait until there is data available in the FIFO buffer or an abort occurred
        while (!i2c->ic_rxflr && !abort_source)
            abort_source = i2c->ic_tx_abrt_source;

        // Stop if aborted
        if (abort_source)
            break;

        // Read bit 8:0 from the data_cmd register into dst
        *dst++ = (uint8_t)i2c->ic_data_cmd;
    }

    return abort_source;
}