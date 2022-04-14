//
// Created by Lorenz on 23/03/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/storage/storage.h"
#include "../include/drivers/sd_spi.h"

/**
 * Simple storage system
 *
 * Block 1 contains:
 *  - magic bytes (0x6B 0x65 0x79 0x6C 0x6F 0x67 0x67 0x65 0x72)
 *  - 3 zero bytes (32-bit align)
 *  - last used block number (32 bits = +- 2TB addressable storage)
 *  - 496 zeroed bytes
 *
 * Block 2-n contains:
 *  - 512 data bytes
 */

// Buffer for temporary holding data for reading/writing to the storage device
static uint8_t block_1[STORAGE_BLOCK_SIZE];


/**
 * Initializes storage instance
 *
 * @param sd_card initialized sd card
 * @param buffer min 512 byte buffer
 * @param buffer_size size of the buffer
 * @return
 */
void storage_init(storage_t *storage, sd_spi_t * sd_card) {
    storage->sd_card = sd_card;
    storage->unsaved_block_buffer_index = 0;
    storage->device_initialized = false;
    if (sd_spi_card_connected(sd_card)) {
        storage_device_init(storage);
    }
}

/**
 * Initialize the storage device and the storage instance with data from the device
 *
 * @param storage
 */
void storage_device_init(storage_t *storage) {
    storage->max_block_count = sd_spi_card_size(storage->sd_card);

    // Read block 1 into the mem buffer
    storage_read_block(storage, block_1, 1);

    // Check if the magic bytes are present
    if(!storage_verify_magic_bytes(block_1)) {
        // Magic bytes are not present, format the device
        storage->last_used_block = 1;
        storage_format_device(storage);
    }
    else {
        // Parse last used block number
        storage->last_used_block = *((uint32_t *) (block_1 + STORAGE_BLOCK1_LAST_BLOCK_OFFSET));
    }
    storage->device_initialized = true;
}

/**
 * Write a 512 byte block to the storage device
 * Wrapper function for sd_spi_write_block
 *
 * @param storage
 * @param buffer
 */
static inline void storage_write_block(storage_t *storage, uint8_t * buffer, uint32_t block) {
    sd_spi_write_block(storage->sd_card, buffer, block);
}

/**
 * Read a 512 byte block from the storage device
 * Wrapper function for sd_spi_read_block
 *
 * @param storage
 * @param buffer
 * @param block
 */
static inline void storage_read_block(storage_t *storage, uint8_t * buffer, uint32_t block) {
    sd_spi_read_block(storage->sd_card, buffer, block);
}

/**
 * Check if the magic bytes are present
 *
 * @param buffer
 * @return true: bytes are present, false: not present
 */
static bool storage_verify_magic_bytes(const uint8_t *buffer) {
    for (int i = 0; i < STORAGE_BLOCK1_MAGIC_BYTES_SIZE; i++) {
        if (buffer[i] != magic_bytes[i]) {
            return false;
        }
    }
    return true;
}

/**
 * Update the last used block value in block 1 on the storage device
 *
 * @param storage
 */
static void storage_update_block_1(storage_t *storage) {
    *((uint32_t*)(block_1 + STORAGE_BLOCK1_LAST_BLOCK_OFFSET)) = storage->last_used_block;
    storage_write_block(storage, block_1, 1);
}

/**
 * Formats the storage device for this storage system
 *
 * @param storage
 */
static void storage_format_device(storage_t *storage) {
    int i = 0;

    // Magic bytes
    for (i; i < STORAGE_BLOCK1_MAGIC_BYTES_SIZE; i++) {
        block_1[i] = magic_bytes[i];
    }

    // 3 zero bytes
    block_1[i++] = 0;
    block_1[i++] = 0;
    block_1[i++] = 0;

    // Last used block
    *((uint32_t*)(block_1 + STORAGE_BLOCK1_LAST_BLOCK_OFFSET)) = 1;
    i += 4;

    // Zero the rest of the block
    memset(block_1, 0, 512);

    // Write the block to the device
    storage_write_block(storage, block_1, 1);
}

/**
 * Stores 1 byte of data in the buffer. This function can be called from an ISR.
 * If the buffer is full, the data will be discarded
 *
 * @param storage
 * @param byte
 * @return true: byte is stored in the buffer, false: buffer is full, the data is not saved
 */
bool storage_store_byte(storage_t *storage, uint8_t byte) {
    if (storage->unsaved_block_buffer_index < STORAGE_UNSAVED_BLOCK_BUFFER_SIZE - 1) {
        storage->unsaved_block_buffer[storage->unsaved_block_buffer_index++] = byte;
        return true;
    }
    return false;
}

/**
 * Storage task, should be called regularly
 *
 * @param storage
 */
void storage_task(storage_t *storage) {
    // Initialize the storage device if this wasn't the case already
    if (storage->device_initialized == false && sd_spi_card_connected(storage->sd_card)){
        storage_device_init(storage);
    }

    // Write the buffer to the storage device if necessary.
    // If the storage device is not yet initialized, the buffer will be overwritten if it contains more than 512 bytes
    if (storage->unsaved_block_buffer_index >= 512) {
        if (storage->device_initialized) {
            // Write the block and update block 1 on the storage device
            storage->last_used_block++;
            storage_write_block(storage, storage->unsaved_block_buffer, storage->last_used_block);
            storage_update_block_1(storage);
        }

        // Reset the block buffer index
        // Check if there is data in the buffer that wasn't in the first 512 bytes
        if(storage->unsaved_block_buffer_index > 512 && STORAGE_UNSAVED_BLOCK_BUFFER_SIZE > 512) {
            // Copy the end of the buffer back to the front
            memcpy(storage->unsaved_block_buffer, storage->unsaved_block_buffer + 512, storage->unsaved_block_buffer_index - 512);
            storage->unsaved_block_buffer_index -= 512;
        }
        else {
            storage->unsaved_block_buffer_index = 0;
        }
    }
}