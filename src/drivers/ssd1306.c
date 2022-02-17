//
// Created by Lorenz on 15/11/2021.
//

#include <stdint.h>
#include <stdbool.h>
#include "../include/drivers/ssd1306.h"
#include "../include/hardware/i2c.h"


ssd1306_t ssd1306_init(i2c_hw_t *i2c, uint8_t address, uint8_t width, uint8_t height, uint8_t * buffer) {
    ssd1306_t ssd1306 = {i2c, address, width, height, buffer};

    // Turn the display off
    ssd1306_display_on(&ssd1306, false);

    // Enable charge pump
    uint8_t cmd_list[] = {SSD1306_COMMAND_CHARGE_PUMP_SETTING, 0x14};
    ssd1306_send_commands(&ssd1306, cmd_list, 2);

    // Set horizontal addressing mode
    ssd1306_set_memory_addressing_mode(&ssd1306, 0x0);

    // Column address 127 mapped to SEG0
    ssd1306_set_segment_remap(&ssd1306, true);

    // Flip vertically
    ssd1306_set_com_output_scan_dir(&ssd1306, true);

    // Set correct OLED panel loyout
    ssd1306_set_com_pins(&ssd1306);

    // Set the duration of the pre-charge period
    ssd1306_set_precharge_period(&ssd1306,0x1, 0xF);

    // Set Vcomh deselect level?

    // Deactivate scrolling
    ssd1306_activate_scrolling(&ssd1306, false);

    // Turn the display on
    ssd1306_display_on(&ssd1306, true);

    return ssd1306;
}

/**
 * Sends a single command
 *
 * @param ssd1306
 * @param command
 */
static inline void ssd1306_send_command(ssd1306_t *ssd1306, uint8_t command) {
    i2c_write(ssd1306->i2c, ssd1306->address, 0x00, 1, true, false);  // Control byte Co and D/C#
    i2c_write(ssd1306->i2c, ssd1306->address, &command, 1, false, true);
}

/**
 * Sends len commands from the command buffer
 *
 * @param ssd1306
 * @param command
 * @param len
 */
static inline void ssd1306_send_commands(ssd1306_t *ssd1306, uint8_t *command, uint32_t len) {
    i2c_write(ssd1306->i2c, ssd1306->address, 0x00, 1, true, false);  // Control byte Co and D/C#
    i2c_write(ssd1306->i2c, ssd1306->address, command, len, false, true);
}

/**
 * Turns the display ON or OFF
 *
 * @param ssd1306
 * @param on
 */
void ssd1306_display_on(ssd1306_t *ssd1306, bool on) {
    if (on)
        ssd1306_send_command(ssd1306, SSD1306_COMMAND_DISPLAY_ON);
    else
        ssd1306_send_command(ssd1306, SSD1306_COMMAND_DISPLAY_OFF);
}

/**
 * Sets the memory addressing mode
 *
 * @param ssd3106
 * @param mode
 */
static inline void ssd1306_set_memory_addressing_mode(ssd1306_t *ssd1306, uint8_t mode) {
    uint8_t cmd_list[2] = {SSD1306_COMMAND_SET_MEM_ADDR_MODE, mode};
    ssd1306_send_commands(ssd1306, cmd_list, 2);
}

/**
 * Changes the mapping between the display data column address and the segment driver
 *
 * @param ssd1306
 * @param remap 1: collum address 127 is mapped to SEG0 | 0: column address 0 is mapped to SEG0
 */
static inline void ssd1306_set_segment_remap(ssd1306_t *ssd1306, bool remap) {
    uint8_t command = SSD1306_COMMAND_SET_SEGMENT_REMAP | (remap ? 1 : 0);
    ssd1306_send_command(ssd1306, command);
}

/**
 * Sets the scan direction of the COM output (flip vertically)
 *
 * @param ssd1306
 * @param remap 1: remapped mode (COM[N-1] - COM0) | 0: normal mode (COM0 - COM[N-1])
 */
static inline void ssd1306_set_com_output_scan_dir(ssd1306_t *ssd1306, bool remap) {
    uint8_t command = SSD1306_COMMAND_SET_COM_OUTPUT_SCAN_DIR;
    if (remap)
        command |= 1<<3;
    ssd1306_send_command(ssd1306, command);
}

/**
 * Sets the COM signals pin configuration to match the OLED panel hardware layout
 *
 * @param ssd1306
 */
static inline void ssd1306_set_com_pins(ssd1306_t *ssd1306) {
    // See Table 10-3: COM Pins Hardware Configuration

    // 0x02 : Disable COM left/right remap + Sequential COM pin config
    // 0x12 : Disable COM left/right remap + Alternative COM pin config
    // 0x22 : Enable COM left/right remap + Sequential COM pin config
    // 0x32 : Enable COM left/right remap + Alternative COM pin config

    uint8_t commands[2] = {SSD1306_COMMAND_SET_COM_PINS, 0x02};
    if(ssd1306->height == 64)
        commands[1] = 0x12;
    ssd1306_send_commands(ssd1306, commands, 2);
}

/**
 * Set the duration of the pre-charge period
 *
 * @param ssd1306
 * @param phase1_period Counted in number of DCLK
 * @param phase2_period Counted in number of DCLK
 */
static inline void ssd1306_set_precharge_period(ssd1306_t *ssd1306, uint8_t phase1_period, uint8_t phase2_period) {
    uint8_t commands[2] = {SSD1306_COMMAND_SET_PRECHARGE_PERIOD, (uint8_t)(phase2_period << 4) & phase1_period};
    ssd1306_send_commands(ssd1306, commands, 2);
}

/**
 * Activate or deactivate scrolling
 *
 * @param ssd1306
 * @param activate
 */
static inline void ssd1306_activate_scrolling(ssd1306_t *ssd1306, bool activate) {
    if (activate)
        ssd1306_send_command(ssd1306, SSD1306_COMMAND_ACTIVATE_SCROLL);
    else
        ssd1306_send_command(ssd1306, SSD1306_COMMAND_DEACTIVATE_SCROLL);
}

/**
 * Send the display buffer
 *
 * @param ssd1306
 */
void ssd1306_display(ssd1306_t *ssd1306) {
    int count = (ssd1306->width * ssd1306->height)/8;
    uint8_t start_byte = SSD1306_COMMAND_SET_DISPLAY_START_LINE;
    i2c_write(ssd1306->i2c, ssd1306->address, &start_byte,1, true, false);
    i2c_write(ssd1306->i2c, ssd1306->address, ssd1306->buffer, count, false, true);
}

/**
 * Clears the display buffer.
 *
 * @param ssd1306
 */
void ssd1306_clear_display(ssd1306_t *ssd1306) {
    int len = (ssd1306->width * ssd1306->height)/8;
    for(int i = 0; i < len; i++) {
        ssd1306->buffer[i] = 0;
    }
}

/**
 * Sets or clears a pixel at a (x,y) coordinate
 *
 * @param ssd1306
 * @param x
 * @param y
 * @param on true: turn the pixel on | false: turn the pixel off
 */
void ssd1306_set_pixel(ssd1306_t *ssd1306, uint8_t x, uint8_t y, bool on) {
    uint32_t buffer_index = x + (y/8) * ssd1306->width;

    if (on) {
        ssd1306->buffer[buffer_index] |= (1 << (y & 7));
    }
    else {
        ssd1306->buffer[buffer_index] &= ~(1 << (y & 7));
    }
}

