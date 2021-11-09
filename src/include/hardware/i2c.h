//
// Created by Lorenz on 7/11/2021.
//

#ifndef KEYLOGGER_I2C_H
#define KEYLOGGER_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "addressmap.h"

// I2C registers (See Table 462)
typedef struct {
    uint32_t volatile ic_con;               // Control register
    uint32_t volatile ic_tar;               // Target address register
    uint32_t volatile ic_sar;               // Slave address register
    uint32_t _undefined0;
    uint32_t volatile ic_data_cmd;          // RX/TX data buffer and command register
    uint32_t volatile ic_ss_scl_hcnt;       // Standard speed I2C clock SCL high count register
    uint32_t volatile ic_ss_scl_lcnt;       // Standard speed I2C clock SCL low count register
    uint32_t volatile ic_fs_scl_hcnt;       // Fast mode (plus) I2c clock SCL high count register
    uint32_t volatile ic_fs_scl_lcnt;       // Fast mode (plus) I2c clock SCL low count register
    uint32_t _undefined1[2];
    uint32_t volatile ic_intr_stat;         // Interrupt status register
    uint32_t volatile ic_intr_mask;         // Interrupt mask register
    uint32_t volatile ic_raw_intr_stat;     // Raw interrupt status register
    uint32_t volatile ic_rx_tl;             // Receive FIFO threshold register
    uint32_t volatile ic_tx_tl;             // Transmit FIFO threshold register
    uint32_t volatile ic_clr_intr;          // Clear combined and individual interrupt register
    uint32_t volatile ic_clr_rx_under;      // Clear RX_UNDER interrupt register
    uint32_t volatile ic_clr_rx_over;       // Clear RX_OVER interrupt register
    uint32_t volatile ic_clr_tx_over;       // Clear TX_OVER interrupt register
    uint32_t volatile ic_clr_rd_req;        // Clear RD_REQ interrupt register
    uint32_t volatile ic_clr_tx_abrt;       // Clear TX_ABRT interrupt register
    uint32_t volatile ic_clr_rx_done;       // Clear RX_DONE interrupt register
    uint32_t volatile ic_clr_activity;      // Clear ACTIVITY interrupt register
    uint32_t volatile ic_clr_stop_det;      // Clear STOP_DET interrupt register
    uint32_t volatile ic_clr_start_det;     // Clear START_DET interrupt register
    uint32_t volatile ic_clr_gen_call;      // Clear GEN_CALL interrupt register
    uint32_t volatile ic_enable;            // Enable register
    uint32_t volatile ic_status;            // Status register
    uint32_t volatile ic_txflr;             // Transmit FIFO level register
    uint32_t volatile ic_rxflr;             // Receive FIFO level register
    uint32_t volatile ic_sda_hold;          // SDA hold time length register
    uint32_t volatile ic_tx_abrt_source;    // Transmit abort source register
    uint32_t volatile ic_slv_data_nack_only;// Generate slave data NACK register
    uint32_t volatile ic_dma_cr;            // DMA control register
    uint32_t volatile ic_dma_tdlr;          // DMA transmit data level register
    uint32_t volatile ic_dma_rdlr;          // DMA receive data level register
    uint32_t volatile ic_sda_setup;         // SDA setup register
    uint32_t volatile ic_ack_general_call;  // ACK general call register
    uint32_t volatile ic_enable_status;     // Enable status register
    uint32_t volatile ic_fs_spklen;         // SS, FS or FM+ spike suppression limit
    uint32_t _undefined2;
    uint32_t volatile ic_clr_restart_det;   // Clear RESTART_DET interrupt register
} i2c_hw_t;

#define i2c0_hw ((i2c_hw_t *const)I2C0_BASE)
#define i2c1_hw ((i2c_hw_t *const)I2C1_BASE)

// IC_CON register (See Table 463)
// Bits
#define I2C_IC_CON_STOP_DET_IF_MASTER_ACTIVE_BIT    (1ul<<10)
#define I2C_IC_CON_RX_FIFO_FULL_HLD_CTRL_BIT        (1ul<<9)
#define I2C_IC_CON_TX_EMPTY_CTRL_BIT                (1ul<<8)
#define I2C_IC_CON_STOP_DET_IF_ADDRESSED_BIT        (1ul<<7)
#define I2C_IC_CON_IC_SLAVE_DISABLE_BIT             (1ul<<6)
#define I2C_IC_CON_IC_RESTART_EN_BIT                (1ul<<5)
#define I2C_IC_CON_IC_10BITADDR_MASTER_BIT          (1ul<<4)
#define I2C_IC_CON_IC_10BITADDR_SLAVE_BIT           (1ul<<3)
#define I2C_IC_CON_SPEED_BITS                       (3ul<<1)
#define I2C_IC_CON_MASTER_MODE_BIT                  (1ul)
// LSB
#define I2C_IC_CON_SPEED_LSB                        1ul
// Values
#define I2C_IC_CON_SPEED_VALUE_SSM                  1ul
#define I2C_IC_CON_SPEED_VALUE_FM                   2ul
#define I2C_IC_CON_SPEED_VALUE_HSM                  3ul

// IC_SDA_HOLD register (See Table 491)
#define I2C_IC_SDA_HOLD_IC_SDA_RX_HOLD_BITS     (15ul<<16)
#define I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS     (15ul)



// Function prototypes
void i2c_init(i2c_hw_t *i2c, uint32_t clk_rate);
static inline void i2c_set_clk_rate(i2c_hw_t *i2c, uint32_t clk_rate);
void i2c_set_slave_mode(i2c_hw_t *i2c, bool slave, uint8_t address);

#endif //KEYLOGGER_I2C_H
