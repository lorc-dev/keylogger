//
// Created by Lorenz on 27/03/2022.
//

#ifndef KEYLOGGER_GRAPHICS_H
#define KEYLOGGER_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>
#include "../../drivers/ssd1306.h"

#define GRAPHICS_FONT   font_8x8_basic

extern char font_8x8_basic[128][8];

typedef struct {
    uint8_t x;
    uint8_t y;
} coordinate_t;

typedef struct {
    ssd1306_t *display;
    bool changed;
    uint8_t width;
    uint8_t height;
    uint8_t cols;
    uint8_t rows;
    struct {
        uint8_t col;
        uint8_t row;
    } cursor_position;
} graphics_display_t;


// Function prototypes
graphics_display_t graphics_init(ssd1306_t *ssd1306_display);
void graphics_draw_pixel(graphics_display_t *display, coordinate_t pos);
void graphics_clear_display(graphics_display_t *display);
void graphics_display(graphics_display_t *display);
void graphics_draw_line(graphics_display_t *display, coordinate_t start, coordinate_t end);
void graphics_draw_horizontal_line(graphics_display_t *display, coordinate_t start, uint8_t length);
void graphics_draw_vertical_line(graphics_display_t *display, coordinate_t start, uint8_t length);
void graphics_draw_char(graphics_display_t *display, coordinate_t pos, char c);
void graphics_print_char(graphics_display_t *display, char c);
void graphics_print_text(graphics_display_t *display, char *str);
void graphics_set_cursor(graphics_display_t *display, uint8_t row, uint8_t col);
coordinate_t graphics_cursor_coordinate(graphics_display_t *display);
void graphics_task(graphics_display_t *display);

#endif //KEYLOGGER_GRAPHICS_H
