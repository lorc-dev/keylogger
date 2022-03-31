//
// Created by Lorenz on 26/02/2022.
//

#ifndef KEYLOGGER_HID_H
#define KEYLOGGER_HID_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/usb.h"

// Keymaps
extern uint8_t usb_hid_keycode_to_ascii_qwerty[104][3];

#define USB_HID_KEY_CAPS_LOCK   0x39

typedef struct {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint16_t bcd_hid;
    uint8_t b_country_code;
    uint8_t b_num_descriptors;
    uint8_t b_report_type;
    uint8_t w_report_length;
} usb_hid_descriptor_t;

typedef struct __attribute__ ((packed)){
        uint8_t left_ctrl : 1;
        uint8_t left_shift : 1;
        uint8_t left_alt : 1;
        uint8_t left_gui : 1;
        uint8_t right_ctrl : 1;
        uint8_t right_shift : 1;
        uint8_t right_alt : 1;
        uint8_t right_gui : 1;
} usb_hid_modifier_keys_bits_t;

// Keyboard input report in boot protocol (See B.1 Protocol 1 of HID1_11)
typedef struct {
    union {
        usb_hid_modifier_keys_bits_t modifier_keys_bits;
        uint8_t modifier_keys;
    };
    uint8_t reserved;
    uint8_t keycodes[6];
} usb_hid_boot_keyboard_input_report_t;

// Keyboard output report in boot protocol (See B.1 Protocol 1 of HID1_11)
typedef struct __attribute__ ((packed)){
        uint8_t numlock : 1;
        uint8_t capslock : 1;
        uint8_t scrollock : 1;
        uint8_t compose : 1;
        uint8_t kana : 1;
        uint8_t constant : 3;
} usb_hid_boot_keyboard_output_report_t;

// Valid values of bRequest (See page 51 of HID1_11)
typedef enum {
    usb_hid_b_request_get_report = 0x01,
    usb_hid_b_request_get_idle = 0x02,
    usb_hid_b_request_get_protocol = 0x03,
    usb_hid_b_request_set_report = 0x09,
    usb_hid_b_request_set_idle = 0x0A,
    usb_hid_b_request_set_protocol = 0x0B
} usb_hid_b_request_t;

typedef struct {
    usb_hid_boot_keyboard_input_report_t prev_report;
    usb_hid_modifier_keys_bits_t pressed_modifiers;
    uint8_t pressed_keys[6];
    bool capslock_enabled;
} usb_hid_keyboard_report_parser_t;

// Function prototypes
bool usb_hid_report_cmp(usb_hid_boot_keyboard_input_report_t *report1, usb_hid_boot_keyboard_input_report_t *report2);
usb_hid_keyboard_report_parser_t hid_report_parser_init(void);
int hid_report_parse(usb_hid_keyboard_report_parser_t *parser, usb_hid_boot_keyboard_input_report_t *report, uint8_t *pressed_keys);

#endif //KEYLOGGER_HID_H
