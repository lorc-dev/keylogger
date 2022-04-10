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

// Report from E.6 Report Descriptor (Keyboard) in HID1_11
#define HID_BOOT_KEYBOARD_REPORT_DESC   {0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06, 0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00, 0x29, 0x65, 0x81, 0x00, 0xC0}
#define USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE     8
#define USB_HID_BOOT_KEYBOARD_OUTPUT_REPORT_SIZE    1

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

// Report types (See page 51 of HID1_11)
typedef enum {
    usb_hid_report_type_input = 1,
    usb_hid_report_type_output,
    usb_hid_report_type_feature,
} usb_hid_report_type_t;

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
