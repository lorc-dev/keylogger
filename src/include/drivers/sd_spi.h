//
// Created by Lorenz on 9/03/2022.
//

#ifndef KEYLOGGER_SD_SPI_H
#define KEYLOGGER_SD_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/spi.h"

typedef enum {
    SDSC,   // 8MB-2GB
    SDHC,   // 4GB-32GB
    SDXC,   // 64GB-2TB
    SDUC    // 4TB-128TB
} sd_version_t;

typedef enum {
    sd_status_disconnected,
    sd_status_connected,
    sd_status_initialized,
    sd_status_released,
} sd_status_t;

typedef struct {
    spi_hw_t *spi;
    uint8_t cs_pin;
    uint8_t detect_pin;
    sd_version_t sd_version;
    sd_status_t status;
} sd_spi_t;

// Commands (See 7.3.1.3 Detailed Command Description)
#define SD_CMD0_GO_IDLE_STATE           0x00    // Software reset
#define SD_CMD1_SEND_OP_COND            0x01    // Initiate initialization process
#define SD_CMD8_SEND_IF_COND            0x08    // Only SDC V2. Check voltage range
#define SD_CMD9_READ_CSD                0x09    // Read CSD register
#define SD_CMD10_SEND_CID               0x0A    // Read CID register
#define SD_CMD12_STOP_TRANSMISSION      0x0C    // Stop to read data
#define SD_CMD16_SET_BLOCKLEN           0x10    // Change R/W block size
#define SD_CMD17_READ_SINGLE_BLOCK      0x11    // Read a block
#define SD_CMD18_READ_MULTIPLE_BLOCK    0x12    // Read multiple blocks
#define SD_CMD24_WRITE_BLOCK            0x18    // Write a block
#define SD_CMD25_WRITE_MULTIPLE_BLOCK   0x19    // Write multiple blocks
#define SD_CMD55_APP_CMD                0x37    // Leading command of ACMD<n> command
#define SD_CMD58_READ_OCR               0x3A    // Read OCR
#define SD_ACMD41_APP_SEND_OP_COND      0x29    // Only SDC. Initiate initialization process

// R1 response (See 7.3.2.1 Format R1)
#define SD_R1_PARAMETER_ERROR_BIT       (1ul << 6)  // Parameter error
#define SD_R1_ADDRESS_ERROR_BIT         (1ul << 5)  // Address error
#define SD_R1_ERASE_SEQ_ERROR_BIT       (1ul << 4)  // Erase sequence error
#define SD_R1_CMD_CRC_ERROR_BIT         (1ul << 3)  // Command CRC error
#define SD_R1_ILLEGAL_CMD_BIT           (1ul << 2)  // Illegal command
#define SD_R1_ERASE_RESET_BIT           (1ul << 1)  // Erase reset
#define SD_R1_IN_IDLE_STATE_BIT         1ul         // In IDLE state

// (See 7.3.3.2 Start Block Tokens and Stop Tran Token)
#define SD_TOKEN_DATA_START_BLOCK       0xFE

// Data response token (See 7.3.3.1 Data Response Token)
#define SD_TOKEN_DATA_RES_BITS          0x1F    // Data response token mask
#define SD_TOKEN_DATA_RES_ACCEPTED      0x05    // Data accepted
#define SD_TOKEN_DATA_RES_CRC_ERROR     0x0B    // Data rejected due to a CRC error
#define SD_TOKEN_DATA_RES_WRITE_ERROR   0x0D    // Data rejected due to a write error


/**
 * Returns if the sdcard is connected and initialized
 *
 * @param sd
 * @return
 */
//static inline bool sd_spi_card_connected(sd_spi_t *sd) {
//    return sd->status == sd_status_initialized;
//}

/**
 * Returns the sd card status
 *
 * @param sd
 * @return
 */
static inline sd_status_t sd_spi_get_status(sd_spi_t *sd) {
    return sd->status;
}

// Function prototypes
void sd_spi_init(sd_spi_t *sd, spi_hw_t * spi_hw, uint8_t cs_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t clk_pin, uint8_t detect_pin);
void sd_spi_disconnect_handler(void);
void sd_spi_release(sd_spi_t *sd);
void sd_spi_task(sd_spi_t *sd);
bool sd_spi_init_card(sd_spi_t *sd);
static uint8_t sd_spi_send_command(sd_spi_t *sd, uint8_t cmd, uint32_t arg);
static uint8_t sd_spi_send_acommand(sd_spi_t *sd, uint8_t cmd, uint32_t arg);
static bool sd_spi_is_busy(sd_spi_t * sd);
static inline void sd_spi_chip_select_high(sd_spi_t *sd);
static inline void sd_spi_chip_select_low(sd_spi_t *sd);
bool sd_spi_read_block(sd_spi_t *sd, uint8_t *dst, uint32_t block);
bool sd_spi_write_block(sd_spi_t *sd, uint8_t *src, uint32_t block);
void sd_spi_read_csd_register(sd_spi_t *sd, uint8_t *dst);
uint32_t sd_spi_card_size(sd_spi_t *sd);

#endif //KEYLOGGER_SD_SPI_H
