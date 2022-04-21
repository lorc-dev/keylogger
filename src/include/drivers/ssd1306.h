//
// Created by Lorenz on 15/11/2021.
//

#ifndef KEYLOGGER_SSD1306_H
#define KEYLOGGER_SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/i2c.h"

// See SSD1306 datasheet
typedef struct {
    i2c_hw_t *i2c;
    uint8_t address;
    uint8_t width;
    uint8_t height;
    uint8_t buffer[1024]; // 128 * 64 / 8
}ssd1306_t;

// Fundamental Commands (See Table 9-1-1)
#define SSD1306_COMMAND_DISPLAY_OFF 0xAE
#define SSD1306_COMMAND_DISPLAY_ON  0xAF
// Scrolling (See Table 9-1-2)
#define SSD1306_COMMAND_DEACTIVATE_SCROLL   0x2E
#define SSD1306_COMMAND_ACTIVATE_SCROLL     0x2F
// Timing & driving scheme setting (See Table 9-1-5)
#define SSD1306_COMMAND_SET_DISPLAY_CLK         0xDF
#define SSD1306_COMMAND_SET_PRECHARGE_PERIOD    0xD9
// Hardware configuration (See Table 9-1-4)
#define SSD1306_COMMAND_SET_MULTIPLEX_RATIO     0xA8
#define SSD1306_COMMAND_SET_DISPLAY_OFFSET      0xD3
#define SSD1306_COMMAND_SET_DISPLAY_START_LINE  0x40  // Uses bit 5-0 for data
#define SSD1306_COMMAND_SET_SEGMENT_REMAP       0xA0
#define SSD1306_COMMAND_SET_COM_OUTPUT_SCAN_DIR 0xC0
#define SSD1306_COMMAND_SET_COM_PINS            0xDA
// Addressing setting (See Table 9-1-3)
#define SSD1306_COMMAND_SET_MEM_ADDR_MODE   0x20
// Charge bumb setting (See Table 2.1-1 Application note)
#define SSD1306_COMMAND_CHARGE_PUMP_SETTING 0x8D

// Function prototypes
void ssd1306_init(ssd1306_t *ssd1306, i2c_hw_t *i2c, uint8_t address, uint8_t width, uint8_t height);
static inline void ssd1306_send_command(ssd1306_t *ssd1306, uint8_t command);
static inline void ssd1306_send_commands(ssd1306_t *ssd1306, uint8_t *command, uint32_t len);
void ssd1306_display_on(ssd1306_t *ssd1306, bool on);
static inline void ssd1306_set_memory_addressing_mode(ssd1306_t *ssd1306, uint8_t mode);
static inline void ssd1306_set_segment_remap(ssd1306_t *ssd1306, bool remap);
static inline void ssd1306_set_com_output_scan_dir(ssd1306_t *ssd1306, bool remap);
static inline void ssd1306_set_com_pins(ssd1306_t *ssd1306);
static inline void ssd1306_set_precharge_period(ssd1306_t *ssd1306, uint8_t phase1_period, uint8_t phase2_period);
static inline void ssd1306_activate_scrolling(ssd1306_t *ssd1306, bool activate);
void ssd1306_display(ssd1306_t *ssd1306);
void ssd1306_clear_display(ssd1306_t *ssd1306);
void ssd1306_set_pixel(ssd1306_t *ssd1306, uint8_t x, uint8_t y, bool on);


#endif //KEYLOGGER_SSD1306_H
