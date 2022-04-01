//
// Created by Lorenz on 27/03/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include "../../include/lib/graphics/graphics.h"
#include "../../include/drivers/ssd1306.h"


//// Coordinate system:
// +-----> X
// |
// |
// v
// Y


/**
 * Initializes a graphics instance
 *
 * @param ssd1306_display initialized ssd1306 display
 * @return
 */
graphics_display_t graphics_init(ssd1306_t *ssd1306_display) {
    graphics_display_t display;

    // Display properties
    display.display = ssd1306_display;
    display.width = ssd1306_display->width;
    display.height = ssd1306_display->height;

    // Text options
    display.cols = display.width / 8;
    display.rows = display.height / 8;
    display.cursor_position.row = 0;
    display.cursor_position.col = 0;

    display.changed = false;

    graphics_clear_display(&display);

    return display;
}

/**
 * Draw 1 pixel.
 * Wrapper function for ssd1306_set_pixel
 *
 * @param display
 * @param x
 * @param y
 */
void graphics_draw_pixel(graphics_display_t *display, coordinate_t pos) {
    if (pos.x < display->width && pos.y < display->height) {
        ssd1306_set_pixel(display->display, pos.x, pos.y, true);
        display->changed = true;
    }
}

/**
 * Clears the display and resets the cursor location
 * Wrapper function for ssd1306_clear_display
 *
 * @param display
 */
void graphics_clear_display(graphics_display_t *display) {
    ssd1306_clear_display(display->display);
    display->cursor_position.row = 0;
    display->cursor_position.col = 0;
}

/**
 * Sends the buffer to the display.
 * Wrapper function for ssd1306_display
 *
 * @param display
 */
void graphics_display(graphics_display_t *display) {
    // TODO: There seems to be a bug in the ssd1306_display function, calling it twice seems to resolve it
    ssd1306_display(display->display);
    ssd1306_display(display->display);
}

/**
 * Draws a line between two coordinates
 * @note The end coordinate must be greater than the start coordinate
 *
 * @param display
 * @param start_x
 * @param start_y
 * @param end_x
 * @param end_y
 */
void graphics_draw_line(graphics_display_t *display, coordinate_t start, coordinate_t end) {
    // TODO: implement draw line function
}

/**
 * Draws a horizontal line
 *
 * @param display
 * @param start_x
 * @param start_y
 * @param length
 */
void graphics_draw_horizontal_line(graphics_display_t *display, coordinate_t start, uint8_t length) {
    for (int x = 0; x < length; x++) {
        graphics_draw_pixel(display, (coordinate_t){start.x + x, start.y});
    }
}

/**
 * Draws a vertical line
 *
 * @param display
 * @param start_x
 * @param start_y
 * @param length
 */
void graphics_draw_vertical_line(graphics_display_t *display, coordinate_t start, uint8_t length) {
    for (int y = 0; y < length; y++) {
        graphics_draw_pixel(display, (coordinate_t){start.x, start.y + y});
    }
}

/**
 * Draw 1 char at the given location
 *
 * @param display
 * @param start_x
 * @param start_y
 * @param c
 */
void graphics_draw_char(graphics_display_t *display, coordinate_t pos, char c) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (GRAPHICS_FONT[c][y] & 1 << x) {
                graphics_draw_pixel(display, (coordinate_t){pos.x + x, pos.y + y});
            }
        }
    }
}

/**
 * Prints a char at the current cursor position. Increases the cursor position by 1 if it is a regular char.
 * The cursor overflows back to the zero position
 *
 * @param display
 * @param c
 */
void graphics_print_char(graphics_display_t *display, char c) {
    if (c == '\n') {
        // Move the cursor to a newline
        display->cursor_position.col = 0;
        if (++display->cursor_position.row >= display->rows) {
            display->cursor_position.row = 0;
        }
    }
    else if (c == '\t') {
        // TODO: add support for tabs
    }
    else if (c == '\r') {
        // Move the cursor to the beginning of the line
        display->cursor_position.col = 0;
    }
    else {
        // Print the char at the cursor position
        graphics_draw_char(display, graphics_cursor_coordinate(display), c);

        // Move the cursor 1 position
        if (++display->cursor_position.col >= display->cols) {
            display->cursor_position.col = 0;
            if (++display->cursor_position.row >= display->rows) {
                display->cursor_position.row = 0;
            }
        }
    }
}

/**
 * Prints a string at the current cursor position. Increases the cursor position.
 *
 * @param display
 * @param str
 */
void graphics_print_text(graphics_display_t *display, char *str) {
    while (*str != 0) {
        graphics_print_char(display, *str);
        str++;
    }
}

/**
 * Sets the cursor at the given location
 *
 * @param display
 * @param row
 * @param col
 */
void graphics_set_cursor(graphics_display_t *display, uint8_t row, uint8_t col) {
    display->cursor_position.row = (row > display->rows) ? (display->rows - 1) : row;
    display->cursor_position.col = (col > display->cols) ? (display->cols - 1) : col;
}

/**
 * Converts the cursor location from (row,col) to the (x,y) coordinate system
 *
 * @param display
 * @return
 */
coordinate_t graphics_cursor_coordinate(graphics_display_t *display) {
    coordinate_t cursor_coordinate;
    cursor_coordinate.x = display->cursor_position.col * 8;
    cursor_coordinate.y = display->cursor_position.row * 8;
    return cursor_coordinate;
}

/**
 * Sends the buffer to the display, if it has changed.
 * This function needs to be called regularly
 *
 * @param display
 */
void graphics_task(graphics_display_t *display) {
    if (display->changed) {
        graphics_display(display);
        display->changed = false;
    }
}