//
// Created by Lorenz on 9/03/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/drivers/sd_spi.h"
#include "../include/hardware/spi.h"
#include "../include/hardware/gpio.h"
#include "../include/hardware/sio.h"
#include "../include/hardware/timer.h"

sd_spi_t *sd_card[2];  // There can only be one active sd card instance per spi controller (2)

/**
 * Initialises an sd card instance
 *
 * @param spi_hw
 * @param cs_pin
 * @param mosi_pin
 * @param miso_pin
 * @param clk_pin
 * @param detect_pin
 */
void sd_spi_init(sd_spi_t *sd, spi_hw_t * spi_hw, uint8_t cs_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t clk_pin, uint8_t detect_pin) {
    sd->spi = spi_hw;
    sd->cs_pin = cs_pin;
    sd->detect_pin = detect_pin;
    sd->status = sd_status_disconnected;

    if (spi_hw == spi0_hw) {
        sd_card[0] = sd;
    }
    else {
        sd_card[1] = sd;
    }

    // Init SPI
    spi_init(spi_hw,400000);

    // Init CS pin
    gpio_set_function(cs_pin,GPIO_FUNC_SIO);
    sio_init(cs_pin);
    sio_set_dir(cs_pin,OUTPUT);
    sd_spi_chip_select_high(sd);

    // Init miso, mosio and clk pin
    gpio_set_function(miso_pin,GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin,GPIO_FUNC_SPI);
    gpio_set_function(clk_pin,GPIO_FUNC_SPI);

    // Init SD card detect pin
    gpio_set_function(detect_pin, GPIO_FUNC_SIO);
    gpio_set_pulldown(detect_pin, false);
    gpio_set_pullup(detect_pin,true);
    sio_init(detect_pin);
    sio_set_dir(detect_pin,INPUT);
    gpio_set_irq_enabled(detect_pin,gpio_irq_event_edge_high, sd_spi_disconnect_handler);
}

/**
 * Sd card disconnect interrupt handler (edge high)
 */
void sd_spi_disconnect_handler(void) {
    // Detect pin:
    //  - high: disconnected
    //  - low:  connected
    for (int i = 0; i < 2; i++) {
        if (sd_card[i] != 0) {
            if (sio_get(sd_card[i]->detect_pin)) {
                sd_card[i]->status = sd_status_disconnected;
            }
        }
    }
}

/**
 * SD task routine
 *
 * @param sd
 */
void sd_spi_task(sd_spi_t *sd) {
    // Detect pin:
    //  - high: disconnected
    //  - low:  connected
    if (sio_get(sd->detect_pin) == false && sd->status == sd_status_disconnected) {
        sd->status = sd_status_connected;
        sd_spi_init_card(sd);
    }
}

/**
 * Initialises the sdcard.
 * Depending on the sdcard, this function can take a couple of seconds to complete
 *
 * @param sd
 * @return true: initialisation successful, false: initialisation failed
 */
bool sd_spi_init_card(sd_spi_t *sd) {
    // Check if the sd card is connected before trying to send anything to it
    if (sd->status != sd_status_connected) {
        return false;
    }

    wait_ms(200);
    // Write min 74 clock cycles with CS high
    for (int i = 0; i < 10; i++) {
        spi_write_byte(sd->spi, 0xff);
    }
    wait_ms(200);
    sd_spi_chip_select_low(sd);

    // Set the card in idle
    while (sd_spi_send_command(sd, SD_CMD0_GO_IDLE_STATE, 0) !=
           SD_R1_IN_IDLE_STATE_BIT); // TODO: add timeout, can take a couple of 100ms

    // Check SD version
    if (sd_spi_send_command(sd, SD_CMD8_SEND_IF_COND, 0x1AA) & SD_R1_ILLEGAL_CMD_BIT) {
        sd->sd_version = SDSC;
    } else {
        sd->sd_version = SDHC;
        // TODO: Add check if it is SDSC v2 (byte addressing / sector addressing)
    }

    // Initialize the card
    uint32_t arg = sd->sd_version == SDHC ? (1ul << 30) : 0;
    while (sd_spi_send_acommand(sd, SD_ACMD41_APP_SEND_OP_COND, arg)); // TODO: add timeout

    sd_spi_chip_select_high(sd);

    sd->status = sd_status_initialized;
    return true;
}

/**
 * Send a command to the sdcard
 *
 * @param sd
 * @param cmd CMD<n>
 * @param arg 32-bit argument
 * @return 1-byte response
 */
static uint8_t sd_spi_send_command(sd_spi_t *sd, uint8_t cmd, uint32_t arg) {
    // TODO: check if not busy?

    // Select the card
    sd_spi_chip_select_high(sd);
    spi_write_byte(sd->spi, 0xFF);
    sd_spi_chip_select_low(sd);
    spi_write_byte(sd->spi, 0xFF);

    // Command
    spi_write_byte(sd->spi, cmd | 0x40);

    // Argument
    spi_write_byte(sd->spi, (uint8_t) (arg >> 24));
    spi_write_byte(sd->spi, (uint8_t) (arg >> 16));
    spi_write_byte(sd->spi, (uint8_t) (arg >> 8));
    spi_write_byte(sd->spi, (uint8_t) arg);

    // CRC
    uint8_t crc = 0xFF;
    switch (cmd) {
        case SD_CMD0_GO_IDLE_STATE:
            crc = 0x95;
            break;
        case SD_CMD8_SEND_IF_COND:
            crc = 0x87; // CRC for CMD8 with arg 0x1AA
            break;
        default:
            crc = 0xFF;
    }
    spi_write_byte(sd->spi, crc);

    // Read response
    int tries_left = 50;    // Temp timout TODO: movo to header file as a #def or add timer timeout (+- 5ms)
    uint8_t response = 0xFF;
    while ((response & 0x80) && (--tries_left > 0))
        spi_read(sd->spi, 0xFF, &response, 1);

    return response;
}

/**
 * Send a ACMD<n> to the sdcard (CMD55 + CMD<n>)
 *
 * @param sd
 * @param cmd ACMD<n>
 * @param arg 32-bit argument
 * @return 1-byte response
 */
static uint8_t sd_spi_send_acommand(sd_spi_t *sd, uint8_t cmd, uint32_t arg) {
    uint8_t response;
    while((response = sd_spi_send_command(sd, SD_CMD55_APP_CMD,0)) != 0x01 && response != 0x0);
    return sd_spi_send_command(sd, cmd, arg);
}

/**
 * Checks if the sdcard is busy.
 *
 * @param sd
 * @return true: busy, false: not busy
 */
static bool sd_spi_is_busy(sd_spi_t *sd) {
    uint8_t buff;
    spi_read(sd->spi, 0xFF, &buff, 1);
    if (buff == 0xFF) {
        return false;
    }
    return true;
}

/**
 * Makes the Chip Select line high
 *
 * @param sd
 */
static inline void sd_spi_chip_select_high(sd_spi_t *sd) {
    sio_put(sd->cs_pin, true);
}

/**
 * Makes the Chip Select line low
 *
 * @param sd
 */
static inline void sd_spi_chip_select_low(sd_spi_t *sd) {
    sio_put(sd->cs_pin, false);
}

/**
 * Read 1 block (512 bytes) at the specified address into the destination buffer
 *
 * @param sd
 * @param dst destination buffer (min 512 bytes)
 * @param block block number (address)
 * @return true: read successful, false: failed
 */
bool sd_spi_read_block(sd_spi_t *sd, uint8_t *dst, uint32_t block) {
    // Check if the sd card is initialized
    if (sd->status != sd_status_initialized) {
        return false;
    }

    // SDSC cards uses byte unit addressing, SDHC uses block unit addressing (512 byte unit)
    if (sd->sd_version == SDSC) {
        block <<= 9;    // * 512
    }

    if (sd_spi_send_command(sd, SD_CMD17_READ_SINGLE_BLOCK, block)) {
        // Error
        // TODO: handle spi sd read block error
        return false;
    }

    // Wait for the data packet
    int tries_left = 4000;    // Temp timout TODO: movo to header file as a #def or add timer timeout (+- 100ms)
    uint8_t response = 0xFF;
    while ((response == 0xFF) && (--tries_left > 0))
        spi_read(sd->spi, 0xFF, &response, 1);

    if (response == SD_TOKEN_DATA_START_BLOCK) {
        // Block size of 512
        spi_read(sd->spi, 0xFF, dst, 512);

        // Read 2 crc bytes
        uint8_t crc[2];
        spi_read(sd->spi, 0xFF, crc, 2);
    }
    return true;
}

/**
 * Writes 1 block (512) of data at the specified address from the source buffer to the sdcard
 *
 * @param sd
 * @param src source buffer (min 512 bytes)
 * @param block block number (address)
 * @return true: write successful, false: failed
 */
bool sd_spi_write_block(sd_spi_t *sd, uint8_t *src, uint32_t block) {
    // Check if the sd card is initialized
    if (sd->status != sd_status_initialized) {
        return false;
    }

    // SDSC cards uses byte unit addressing, SDHC uses block unit addressing (512 byte unit)
    if (sd->sd_version == SDSC) {
        block <<= 9;    // * 512
    }

    if (sd_spi_send_command(sd, SD_CMD24_WRITE_BLOCK, block)) {
        // error
        // TODO: handle spi sd write block error
        return false;
    }

    // Send token
    spi_write_byte(sd->spi, SD_TOKEN_DATA_START_BLOCK);
    // Send data
    spi_write(sd->spi, src, 512);
    // Send CRC
    spi_write_byte(sd->spi, 0xFF);
    spi_write_byte(sd->spi, 0xFF);

    uint8_t status;
    spi_read(sd->spi, 0xFF, &status, 1);

    if((status & SD_TOKEN_DATA_RES_BITS) != SD_TOKEN_DATA_RES_ACCEPTED){
        // error    TODO: handle spi sd write block error
    }

   // Wait for flash programming to complete
   while(sd_spi_is_busy(sd));

   sd_spi_chip_select_high(sd);

   return true;
}

/**
 * Read the CSD register
 *
 * @param sd
 * @param dst destination buffer (min 16 bytes)
 */
void sd_spi_read_csd_register(sd_spi_t *sd, uint8_t *dst) {
    // TODO: implement read csd register
    // Send cmd9

    // Wait for data block token

    // Read 16 bytes data into dst buffer

    // Read 2 CRC bytes

    sd_spi_chip_select_high(sd);
}

/**
 * Get the size of the sdcard
 *
 * @param sd
 * @return number of 512 byte data blocks
 */
uint32_t sd_spi_card_size(sd_spi_t *sd) {
    // TODO: implement read card size

    // Read CSD register

    // Check CSD version

    return 0;
}