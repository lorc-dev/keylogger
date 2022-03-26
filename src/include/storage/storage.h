//
// Created by Lorenz on 23/03/2022.
//

#ifndef KEYLOGGER_STORAGE_H
#define KEYLOGGER_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "../drivers/sd_spi.h"

#define STORAGE_BLOCK1_MAGIC_BYTES_SIZE 9
#define STORAGE_BLOCK1_LAST_BLOCK_OFFSET 12

static const uint8_t magic_bytes[STORAGE_BLOCK1_MAGIC_BYTES_SIZE] = {0x6B, 0x65, 0x79, 0x6C, 0x6F, 0x67, 0x67, 0x65, 0x72};

typedef struct {
    sd_spi_t * sd_card;
    uint64_t last_used_block;
    uint8_t * unsaved_block_buffer;
    uint16_t unsaved_block_buffer_index;
    uint16_t unsaved_block_buffer_size;
    uint32_t max_block_count;
} storage_t;


// Function prototypes
storage_t storage_init(sd_spi_t * sd_card, uint8_t * buffer, uint16_t buffer_size);
static inline void storage_write_block(storage_t *storage, uint8_t * buffer, uint32_t block);
static inline void storage_read_block(storage_t *storage, uint8_t * buffer, uint32_t block);
static bool storage_verify_magic_bytes(uint8_t *buffer);
static void storage_update_block_1(storage_t *storage);
static void storage_format_device(storage_t *storage);
bool storage_store_byte(storage_t *storage, uint8_t byte);
void storage_task(storage_t *storage);

#endif //KEYLOGGER_STORAGE_H
