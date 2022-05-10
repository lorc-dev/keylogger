//
// Created by Lorenz on 12/04/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "../include/ui/ui.h"
#include "../include/lib/graphics/graphics.h"
#include "../include/hardware/gpio.h"
#include "../include/hardware/sio.h"
#include "../include/hardware/timer.h"

#include "../include/storage/storage.h"
#include "../include/hardware/usb.h"
#include "../include/drivers/usb_hid.h"

ui_t *active_ui;    // There can only be one UI active

/**
 * Init a user interface instance
 *
 * @param ui
 * @param display
 * @param left_button_pin
 * @param right_button_pin
 * @param storage
 * @param keyboard
 */
void ui_init(ui_t *ui, graphics_display_t *display, uint8_t left_button_pin, uint8_t right_button_pin, storage_t *storage, usb_device_t *keyboard, usb_hid_keyboard_report_parser_t *parser) {
    active_ui = ui;

    ui->display = display;
    ui->selected_menu_item = 0;
    ui->left_button_pin = left_button_pin;
    ui->right_button_pin = right_button_pin;

    // Devices
    ui->devices.storage = storage;
    ui->devices.keyboard = keyboard;
    ui->devices.parser = parser;

    // Menu
    ui->buttons.button_right_pressed = false;
    ui->buttons.button_left_pressed = false;
    memset(ui->menu_data_changed, true, UI_MENU_ITEMS);
    ui_menu_data_init(ui);

    // Buttons
    // Left Button
    gpio_set_function(left_button_pin, GPIO_FUNC_SIO);
    gpio_set_pulldown(left_button_pin, false);
    gpio_set_pullup(left_button_pin,true);
    sio_init(left_button_pin);
    sio_set_dir(left_button_pin,INPUT);
    gpio_set_irq_enabled(left_button_pin,gpio_irq_event_edge_low, ui_on_left_button_handler);
    // Right button
    gpio_set_function(right_button_pin, GPIO_FUNC_SIO);
    gpio_set_pulldown(right_button_pin, false);
    gpio_set_pullup(right_button_pin,true);
    sio_init(right_button_pin);
    sio_set_dir(right_button_pin,INPUT);
    gpio_set_irq_enabled(right_button_pin,gpio_irq_event_edge_low, ui_on_right_button_handler);
}

/**
 * Left button press interrupt handler
 */
static void ui_on_left_button_handler(void) {
    wait_us(500);   // TODO: Implement a better way to debounce the buttons
    if(!sio_get(active_ui->left_button_pin)) {
        active_ui->buttons.button_left_pressed = true;
    }
}

/**
 * Right button press interrupt handler
 */
static void ui_on_right_button_handler(void) {
    wait_us(500);   // TODO: Implement a better way to debounce the buttons
    if(!sio_get(active_ui->right_button_pin)) {
        active_ui->buttons.button_right_pressed = true;
    }
}

/**
 * UI task, should be called regularly
 *
 * @param ui
 */
void ui_task(ui_t *ui) {
    // Only one button can be pressed at the same time, if both are pressed assume only the left one was pressed
    if (ui->buttons.button_left_pressed) {
        // Move to the next menu item. If we are at the end, loop back around
        if (++ui->selected_menu_item >= UI_MENU_ITEMS)
            ui->selected_menu_item = 0;
        ui_menu_display(ui, ui->selected_menu_item, false);
    }
    else if (ui->buttons.button_right_pressed) {
        ui_menu_display(ui, ui->selected_menu_item, true);
    }
    else if (ui->menu_data_changed[ui->selected_menu_item]) {
        ui_menu_display(ui, ui->selected_menu_item, false);
        ui->menu_data_changed[ui->selected_menu_item] = false;
    }

    ui->buttons.button_right_pressed = false;
    ui->buttons.button_left_pressed = false;
}

/**
 * UI data changed event handler
 *
 * @param source
 */
void ui_data_changed_event_handler(ui_data_source_t source) {
    switch (source) {
        case ui_data_source_keyboard:
            active_ui->menu_data_changed[ui_menu_item_keyboard_status] = true;
            break;
        case ui_data_source_storage:
            active_ui->menu_data_changed[ui_menu_item_sd_card_status] = true;
            active_ui->menu_data_changed[ui_menu_item_format_sd_card] = true;
            active_ui->menu_data_changed[ui_menu_item_release_sd_card] = true;
            break;
    }
}

/**
 * Initialize the data for each menu item
 *
 * @param ui
 */
static void ui_menu_data_init(ui_t *ui) {
    // Sd card status menu
    memset(ui->menu_items.sd_card_status.data_used_text, 0, 9);

    // Keyboard status menu
    memset(ui->menu_items.keyboard_status.vid_text, 0, 5);
    memset(ui->menu_items.keyboard_status.pid_text, 0, 5);

    // User stats menu
    memset(ui->menu_items.user_stats.wpm_text, 0, 4);
    ui->menu_items.user_stats.wpm = 0;

    // Live output menu
    memset(ui->menu_items.live_output.chars, ' ', UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT);

    // Release SD card menu

    // Format SD card menu

    // Keymap selection menu
    ui->menu_items.keymap_selection.keymap = hid_get_selected_keymap(ui->devices.parser);
}

/**
 * Displays the selected menu
 *
 * @param ui
 * @param menu
 * @param action_key_pressed
 */
static void ui_menu_display(ui_t *ui, ui_menu_item_t menu, bool action_key_pressed) {
    switch (menu) {
        case ui_menu_item_sd_card_status:
            ui_menu_sd_card_status(ui);
            break;
        case ui_menu_item_keyboard_status:
            ui_menu_keyboard_status(ui);
            break;
        case ui_menu_item_user_stats:
            ui_menu_user_stats(ui);
            break;
        case ui_menu_item_live_output:
            ui_menu_live_output(ui);
            break;
        case ui_menu_item_release_sd_card:
            ui_menu_release_sd_card(ui, action_key_pressed);
            break;
        case ui_menu_item_format_sd_card:
            ui_menu_format_sd_card(ui, action_key_pressed);
            break;
        case ui_menu_item_keymap_selection:
            ui_menu_keymap_selection(ui, action_key_pressed);
            break;
        case UI_MENU_ITEMS:
            break;
    }
}

/**
 * Display the splashscreen
 *
 * @param ui
 */
void ui_menu_loading_screen(ui_t *ui) {
    uint8_t loading_screen[512] = {
            0x00,0x00,0x80,0xc0,0xf0,0xf8,0xf8,0xfc,0xfe,0xfe,0xff,
             0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
             0xfe,0xfe,0xfc,0xfc,0xf8,0xf0,0xe0,0xc0,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x40,0x00,0x00,0x00,0x3e,0x3e,0x3e,0x3e,
             0x3e,0x00,0x60,0xf0,0xf8,0xf8,0xf0,0x60,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0xfe,0xff,0xff,0xff,0xff,0x3f,0x0f,0x0f,
             0x07,0x07,0x07,0x07,0x0f,0x1f,0xff,0xff,0xff,
             0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
             0xff,0xff,0xff,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,
             0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,
             0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,
             0xfe,0xfe,0xfc,0xfc,0xf8,0xf0,0xe0,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x81,0x3d,0x3c,0x3c,
             0x3c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x1f,0x7f,0xff,0xff,0xff,0xff,
             0xfc,0xf8,0xf0,0xf0,0xf0,0xf8,0xf8,0xfc,0xff,
             0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
             0xff,0xff,0xff,0xff,0x7f,0x3f,0x3f,0x3f,0x3f,
             0x3f,0x3f,0x3f,0x1f,0x0f,0x0f,0x0f,0x0f,0x0f,
             0x07,0x03,0x03,0x07,0x0f,0x0f,0x0f,0xc7,0xc3,
             0x03,0x07,0x8f,0xcf,0x4f,0x07,0x03,0x01,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x40,0xc0,0xc0,
             0x00,0x00,0x00,0x02,0x07,0x02,0x00,0x00,0x00,
             0x1c,0x1c,0x1c,0x00,0x00,0x02,0x07,0x0f,0x07,
             0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,
             0x07,0x0f,0x1f,0x1f,0x3f,0x3f,0x7f,0x7f,0x7f,
             0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x3f,0x3f,0x3f,
             0x1f,0x1f,0x0f,0x07,0x03,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
             0x1f,0x1f,0x06,0x0f,0x1d,0x18,0x10,0x00,0x0e,
             0x1f,0x15,0x15,0x15,0x17,0x06,0x00,0x07,0x2f,
             0x28,0x28,0x28,0x3f,0x1f,0x00,0x00,0x10,0x10,
             0x1f,0x1f,0x10,0x10,0x00,0x0e,0x1f,0x11,0x11,
             0x11,0x1f,0x0e,0x00,0x06,0x2f,0x29,0x29,0x29,
             0x3f,0x1f,0x00,0x06,0x2f,0x29,0x29,0x29,0x3f,
             0x1f,0x00,0x0e,0x1f,0x15,0x15,0x15,0x17,0x06,
             0x00,0x00,0x1f,0x1f,0x02,0x01,0x01,0x01,0x00,
             0x00,0x00,0x00,0x00,0x00,0x00
    };
    memcpy(ui->display->display->buffer, loading_screen, 512);  // TODO: Make an interface for this in the graphics lib and/or the ssd1306 driver
    graphics_display(ui->display);
}

/**
 * Display the SD card status menu
 *
 * @param ui
 */
static void ui_menu_sd_card_status(ui_t *ui) {
    coordinate_t text_pos;

    // Recalculate if data has changed
    if (ui->menu_data_changed[ui_menu_item_sd_card_status]) {
        ui_bytes_to_str(storage_get_used_blocks(ui->devices.storage) * 512, ui->menu_items.sd_card_status.data_used_text);
        ui->menu_data_changed[ui_menu_item_sd_card_status] = false;
    }

    // Clear the display before drawing anything new on it
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "SD card info");

    // Connected status
    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;
    if (storage_get_device_initialized(ui->devices.storage)) {
        graphics_draw_text(ui->display, text_pos, "Connected");

        // Data used (only when the card is connected)
        text_pos.x = 0;
        text_pos.y += GRAPHICS_FONT_CHAR_HEIGHT + 1;
        graphics_draw_text(ui->display, text_pos, "Used: ");

        text_pos.x += 6 * GRAPHICS_FONT_CHAR_WIDTH;
        graphics_draw_text(ui->display, text_pos, ui->menu_items.sd_card_status.data_used_text);
    }
    else {
        graphics_draw_text(ui->display, text_pos, "Not connected");
    }
}

/**
 * Display the keyboard status menu
 *
 * @param ui
 */
static void ui_menu_keyboard_status(ui_t *ui) {
    coordinate_t text_pos;

    // Recalculate if data has changed
    if (ui->menu_data_changed[ui_menu_item_keyboard_status]) {
        itoa(usb_get_vid(ui->devices.keyboard), ui->menu_items.keyboard_status.vid_text, 16);
        itoa(usb_get_pid(ui->devices.keyboard), ui->menu_items.keyboard_status.pid_text, 16);
        ui->menu_data_changed[ui_menu_item_keyboard_status] = false;
    }

    // Clear the display before drawing anything new on it
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "Keyboard");

    // Connected status
    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;
    if (usb_get_device_connected(ui->devices.keyboard)) {
        graphics_draw_text(ui->display, text_pos, "Connected");

        // VID & PID (only when the keyboard is connected)
        text_pos.x = 0;
        text_pos.y += GRAPHICS_FONT_CHAR_HEIGHT + 1;
        graphics_draw_text(ui->display, text_pos, "V/PID: ");
        text_pos.x += 7 * GRAPHICS_FONT_CHAR_WIDTH;
        graphics_draw_text(ui->display, text_pos, ui->menu_items.keyboard_status.vid_text);
        text_pos.x += 5 * GRAPHICS_FONT_CHAR_WIDTH;
        graphics_draw_text(ui->display, text_pos, ui->menu_items.keyboard_status.pid_text);
    }
    else {
        graphics_draw_text(ui->display, text_pos, "Not connected");
    }
}

/**
 * Display the user stats menu
 *
 * @param ui
 */
static void ui_menu_user_stats(ui_t *ui) {
    coordinate_t text_pos;

    // Recalculate if data has changed
    if (ui->menu_data_changed[ui_menu_item_user_stats]) {
        itoa(ui->menu_items.user_stats.wpm, ui->menu_items.user_stats.wpm_text, 10);
        ui->menu_data_changed[ui_menu_item_user_stats] = false;
    }

    // Clear the display before drawing anything new on it
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "User stats");

    // Words per minute
    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;
    graphics_draw_text(ui->display, text_pos, "wpm: ");
    text_pos.x += 5 * GRAPHICS_FONT_CHAR_WIDTH;
    graphics_draw_text(ui->display, text_pos, ui->menu_items.user_stats.wpm_text);
}

/**
 * Display the live output menu
 *
 * @param ui
 */
static void ui_menu_live_output(ui_t *ui) {
    coordinate_t text_pos;

    // Clear the display before drawing anything new on it
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "Live input");

    // Latest characters
    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;
    char latest_chars[UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT + 1];
    memcpy(latest_chars, ui->menu_items.live_output.chars, UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT);
    latest_chars[UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT] = '\0';
    graphics_draw_text(ui->display, text_pos, latest_chars);
}

/**
 * Display the release SD card menu
 *
 * @param ui
 * @param action_key_pressed
 */
static void ui_menu_release_sd_card(ui_t *ui, bool action_key_pressed) {
    coordinate_t text_pos;
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "Release SD card");

    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;

    switch (sd_spi_get_status(ui->devices.storage->sd_card)){
        case sd_status_disconnected:
            graphics_draw_text(ui->display, text_pos, "Disconnected");
            break;
        case sd_status_connected:
            break;
        case sd_status_initialized:
            if (action_key_pressed) {
                storage_release(ui->devices.storage);
            }
            else {
                graphics_draw_text(ui->display, text_pos, "Press OK");
            }
            break;
        case sd_status_released:
            graphics_draw_text(ui->display, text_pos, "Remove card");
            break;
    }
}

/**
 * Display the format SD card menu
 *
 * @param ui
 * @param action_key_pressed
 */
static void ui_menu_format_sd_card(ui_t *ui, bool action_key_pressed) {
    coordinate_t text_pos;
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "Format SD card");

    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;
    if (action_key_pressed) {
        graphics_draw_text(ui->display, text_pos, "Formatting...");
        storage_format(ui->devices.storage);
    }
    else if (storage_get_device_initialized(ui->devices.storage)) {
        graphics_draw_text(ui->display, text_pos, "Press OK");
    }
    else {
        graphics_draw_text(ui->display, text_pos, "Not connected");
    }
}

/**
 * Display the keymap selection menu
 *
 * @param ui
 * @param action_key_pressed
 */
static void ui_menu_keymap_selection(ui_t *ui, bool action_key_pressed) {
    coordinate_t text_pos;

    if (action_key_pressed) {
        switch(ui->menu_items.keymap_selection.keymap) {
            case hid_keymap_azerty:
                hid_report_parser_set_keymap(ui->devices.parser, hid_keymap_qwerty);
                break;
            case hid_keymap_qwerty:
                hid_report_parser_set_keymap(ui->devices.parser, hid_keymap_azerty);
                break;
        }
        ui->menu_items.keymap_selection.keymap = hid_get_selected_keymap(ui->devices.parser);
    }

    // Clear the display before drawing anything new on it
    graphics_clear_display(ui->display);

    // Menu title
    graphics_draw_title(ui->display, "Keymap");

    text_pos.x = 0;
    text_pos.y = GRAPHICS_FONT_CHAR_HEIGHT + 2;
    switch(ui->menu_items.keymap_selection.keymap) {
        case hid_keymap_azerty:
            graphics_draw_text(ui->display, text_pos, "< AZERTY >");
            break;
        case hid_keymap_qwerty:
            graphics_draw_text(ui->display, text_pos, "< QWERTY >");
            break;
    }
}

/**
 * Convert a number to a string with a byte unit
 * @example 2068 -> "2 KiB"
 * @example 694  -> "694 B"
 *
 * @param bytes
 * @param str 4 (chars) + 1 (space) + 3 (unit) + 1(null byte) = min 9 bytes buffer
 */
static void ui_bytes_to_str(uint32_t bytes, char *str) {
    const uint8_t unit_prefix[4] = {'K', 'M', 'G', 'T'};

    // Find the correct unit prefix so that the final number is not greater than 1024 (4 chars)
    int8_t prefix_index = -1;
    while (bytes >= 1024 && prefix_index < 3) {
        prefix_index++;
        bytes /= 1024;
    }

    itoa(bytes, str, 10);
    uint8_t i = strlen(str);
    str[i++] = ' ';
    if (prefix_index >= 0) {
        str[i++] = unit_prefix[prefix_index];
        str[i++] = 'i';
    }
    str[i++] = 'B';
    str[i] = '\0';
}