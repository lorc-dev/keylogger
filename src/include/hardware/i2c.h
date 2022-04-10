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


// IC_DATA_CMD register (See Table 466)
#define I2C_IC_DATA_CMD_FIRST_DATA_BYTE_BIT (1ul<<11)
#define I2C_IC_DATA_CMD_RESTART_BIT         (1ul<<10)
#define I2C_IC_DATA_CMD_STOP_BIT            (1ul<<9)
#define I2C_IC_DATA_CMD_CMD_BIT             (1ul<<8)
#define I2C_IC_DATA_CMD_DAT_BITS            (0xff)
// LSB
#define I2C_IC_DATA_CMD_FIRST_DATA_BYTE_LSB 11ul
#define I2C_IC_DATA_CMD_RESTART_LSB         10ul
#define I2C_IC_DATA_CMD_STOP_LSB            9ul
#define I2C_IC_DATA_CMD_CMD_LSB             8ul
#define I2C_IC_DATA_CMD_DAT_LSB             0ul
// Values
#define I2C_IC_DATA_CMD_CMD_VALUE_MASTER_WRITE  0ul
#define I2C_IC_DATA_CMD_CMD_VALUE_MASTER_READ   1ul


// IC_RAW_INTR_STAT register (See Table 473)
#define I2C_IC_RAW_INTR_STAT_RESTART_DET_BIT    (1ul<<12)
#define I2C_IC_RAW_INTR_STAT_GEN_CALL_BIT       (1ul<<11)
#define I2C_IC_RAW_INTR_STAT_START_DET_BIT      (1ul<<10)
#define I2C_IC_RAW_INTR_STAT_STOP_DET_BIT       (1ul<<9)
#define I2C_IC_RAW_INTR_STAT_ACTIVITY_BIT       (1ul<<8)
#define I2C_IC_RAW_INTR_STAT_RX_DONE_BIT        (1ul<<7)
#define I2C_IC_RAW_INTR_STAT_TX_ABRT_BIT        (1ul<<6)
#define I2C_IC_RAW_INTR_STAT_RD_REQ_BIT         (1ul<<5)
#define I2C_IC_RAW_INTR_STAT_TX_EMPTY_BIT       (1ul<<4)
#define I2C_IC_RAW_INTR_STAT_TX_OVER_BIT        (1ul<<3)
#define I2C_IC_RAW_INTR_STAT_RX_FULL_BIT        (1ul<<2)
#define I2C_IC_RAW_INTR_STAT_RX_OVER_BIT        (1ul<<1)
#define I2C_IC_RAW_INTR_STAT_RX_UNDER_BIT       (1ul)

// IC_INTR_MASK register (See Table 472)
#define I2C_IC_INTR_MASK_RESTART_DET_BIT    (1ul<<12)
#define I2C_IC_INTR_MASK_GEN_CALL_BIT       (1ul<<11)
#define I2C_IC_INTR_MASK_START_DET_BIT      (1ul<<10)
#define I2C_IC_INTR_MASK_STOP_DET_BIT       (1ul<<9)
#define I2C_IC_INTR_MASK_ACTIVITY_BIT       (1ul<<8)
#define I2C_IC_INTR_MASK_RX_DONE_BIT        (1ul<<7)
#define I2C_IC_INTR_MASK_TX_ABRT_BIT        (1ul<<6)
#define I2C_IC_INTR_MASK_RD_REQ_BIT         (1ul<<5)
#define I2C_IC_INTR_MASK_TX_EMPTY_BIT       (1ul<<4)
#define I2C_IC_INTR_MASK_TX_OVER_BIT        (1ul<<3)
#define I2C_IC_INTR_MASK_RX_FULL_BIT        (1ul<<2)
#define I2C_IC_INTR_MASK_RX_OVER_BIT        (1ul<<1)
#define I2C_IC_INTR_MASK_RX_UNDER_BIT       (1ul)


// IC_SDA_HOLD register (See Table 491)
#define I2C_IC_SDA_HOLD_IC_SDA_RX_HOLD_BITS     (0xfffful<<16)
#define I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS     (0xfffful)


// IC_TX_ABRT_SOURCE register (See Table 492)
#define I2C_IC_TX_ABRT_SOURCE_TX_FLUSH_CNT_BITS         (0x1fful<<23)
#define I2C_IC_TX_ABRT_SOURCE_ABRT_USER_ABRT_BIT        (1ul<<16)   // Transfer abort detected by master
#define I2C_IC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_BIT       (1ul<<15)   // Slave trying to transmit in read mode
#define I2C_IC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_BIT      (1ul<<14)   // Slave lost arbitration to remote master
#define I2C_IC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_BIT  (1ul<<13)   // Slave flushes existing data in TX FIFO upon read cmd
#define I2C_IC_TX_ABRT_SOURCE_ABRT_LOST_BIT             (1ul<<12)   // Master or slave-transmitter lost arbitration
#define I2C_IC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_BIT       (1ul<<11)   // User initiating master operation when master disabled
#define I2C_IC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_BIT   (1ul<<10)   // Master trying to read in 10B addr mode when RESTART disabled
#define I2C_IC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_BIT    (1ul<<9)    // User trying to send START byte when RESTART disabled
#define I2C_IC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_BIT       (1ul<<8)    // User trying to switch master to HS mode when RESTART disabled
#define I2C_IC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_BIT     (1ul<<7)    // ACK detected for START byte
#define I2C_IC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_BIT        (1ul<<6)    // HS master code ACKed in HS mode
#define I2C_IC_TX_ABRT_SOURCE_ABRT_GCALL_READ_BIT       (1ul<<5)    // General Call is followed by read from bus
#define I2C_IC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_BIT      (1ul<<4)    // General Call not ACKed by any slave
#define I2C_IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_BIT     (1ul<<3)    // Transmitted data not ACKed by addressed slave
#define I2C_IC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_BIT    (1ul<<2)    // Byte 2 of 10B address not ACKed by any slave
#define I2C_IC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_BIT    (1ul<<1)    // Byte 1 of 10B address not ACKed by anny slave
#define I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_BIT    (1ul)       // NOACK for 7-bit address

typedef enum {
    i2c_slave_receive,  // Data from master available
    i2c_slave_request,  // Master is requesting data
    i2c_slave_finish,   // Master has sent the stop or restart signal
} i2c_slave_event_t;

typedef void (*i2c_slave_handler_t)(i2c_hw_t *i2c, i2c_slave_event_t event);

typedef struct {
    i2c_hw_t *i2c;
    bool transfer_in_progress;
    i2c_slave_handler_t handler;
} i2c_slave_t;


// Function prototypes
void i2c_init(i2c_hw_t *i2c, uint32_t clk_rate);
static inline void i2c_set_clk_rate(i2c_hw_t *i2c, uint32_t clk_rate);
void i2c_set_slave_mode(i2c_hw_t *i2c, bool slave, uint8_t address);
static inline void i2c_set_target_address(i2c_hw_t *i2c, uint8_t address);
uint32_t i2c_write(i2c_hw_t *i2c, uint8_t address, uint8_t *src, uint32_t len, bool start, bool stop);
uint32_t i2c_read(i2c_hw_t *i2c, uint8_t address, uint8_t *dst, uint32_t len, bool start, bool stop);
void i2c_slave_init(i2c_hw_t *i2c, uint8_t address, i2c_slave_handler_t handler);
void i2c_slave_irq(i2c_slave_t *slave);
static inline void i2c_slave_finish_transfer(i2c_slave_t *slave);
uint8_t i2c_slave_read_byte(i2c_hw_t *i2c);
void i2c_slave_write_byte(i2c_hw_t *i2c, uint8_t value);
bool i2c_is_readable(i2c_hw_t *i2c);
bool i2c_is_writable(i2c_hw_t *i2c);


#endif //KEYLOGGER_I2C_H
