//
// Created by Lorenz on 12/04/2022.
//

#ifndef KEYLOGGER_UI_H
#define KEYLOGGER_UI_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../lib/graphics/graphics.h"

#include "../storage/storage.h"
#include "../hardware/usb.h"
#include "../drivers/usb_hid.h"


// Live output menu
#define UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT 10  // Number of characters to view in the live output menu

// Menu items and the order in which these are displayed
typedef enum {
    ui_menu_item_sd_card_status,
    ui_menu_item_keyboard_status,
    ui_menu_item_user_stats,
    ui_menu_item_live_output,
    ui_menu_item_release_sd_card,
    ui_menu_item_format_sd_card,
    ui_menu_item_keymap_selection,
    UI_MENU_ITEMS
} ui_menu_item_t;

typedef enum {
    ui_data_source_keyboard,
    ui_data_source_storage,
} ui_data_source_t;

typedef struct {
    graphics_display_t *display;
    ui_menu_item_t selected_menu_item;
    uint8_t left_button_pin;
    uint8_t right_button_pin;

    // Buttons
    struct {
        bool button_left_pressed;
        bool button_right_pressed;
    } buttons;

    // Menu data changed
    bool menu_data_changed[UI_MENU_ITEMS];

    // Devices / Drivers
    struct {
        storage_t *storage;
        usb_device_t *keyboard;
        usb_hid_keyboard_report_parser_t  *parser;
    } devices;

    // Menu item data
    struct {
        // SD card status
        struct {
            char data_used_text[9];
        } sd_card_status;

        // Keyboard status
        struct {
            char vid_text[5];
            char pid_text[5];
        } keyboard_status;

        // User status
        struct {
            char wpm_text[4];
            uint8_t wpm;
        } user_stats;

        // Live output
        struct {
            uint8_t chars[UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT];
        } live_output;

        // Release sd card
        struct {

        } release_sd_card;

        // Format sd card
        struct {

        } format_sd_card;

        // Keymap selection
        struct {
            hid_keymap_t keymap;
        } keymap_selection;
    } menu_items;
} ui_t;


/**
 * Data setter for the user stats menu
 *
 * @param ui
 * @param wpm
 */
static inline void ui_menu_user_stats_set_data(ui_t *ui, uint8_t wpm) {
    // Words per minute
    ui->menu_data_changed[ui_menu_item_user_stats] = true;
    ui->menu_items.user_stats.wpm = wpm;
}

/**
 * Data setter for the live output menu
 *
 * @param ui
 * @param c
 */
static inline void ui_menu_live_output_set_data(ui_t *ui, char c) {
    ui->menu_data_changed[ui_menu_item_live_output] = true;
    memmove(ui->menu_items.live_output.chars, &ui->menu_items.live_output.chars[1], UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT - 1);
    ui->menu_items.live_output.chars[UI_MENU_ITEM_LIVE_OUTPUT_CHAR_COUNT - 1] = c;
}

// Function prototypes
void ui_init(ui_t *ui, graphics_display_t *display, uint8_t left_button_pin, uint8_t right_button_pin, storage_t *storage, usb_device_t *keyboard, usb_hid_keyboard_report_parser_t *parser);
static void ui_on_left_button_handler(void);
static void ui_on_right_button_handler(void);
void ui_task(ui_t *ui);
void ui_data_changed_event_handler(ui_data_source_t source);
void ui_menu_loading_screen(ui_t *ui);
static void ui_menu_data_init(ui_t *ui);
static void ui_menu_display(ui_t *ui, ui_menu_item_t menu, bool action_key_pressed);
static void ui_menu_sd_card_status(ui_t *ui);
static void ui_menu_keyboard_status(ui_t *ui);
static void ui_menu_user_stats(ui_t *ui);
static void ui_menu_live_output(ui_t *uig);
static void ui_menu_release_sd_card(ui_t *ui, bool action_key_pressed);
static void ui_menu_format_sd_card(ui_t *ui, bool action_key_pressed);
static void ui_menu_keymap_selection(ui_t *ui, bool action_key_pressed);
static void ui_bytes_to_str(uint32_t bytes, char *str);

#endif //KEYLOGGER_UI_H
