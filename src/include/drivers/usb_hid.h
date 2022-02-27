//
// Created by Lorenz on 26/02/2022.
//

#ifndef KEYLOGGER_HID_H
#define KEYLOGGER_HID_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/usb.h"

typedef struct {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint16_t bcd_hid;
    uint8_t b_country_code;
    uint8_t b_num_descriptors;
    uint8_t b_report_type;
    uint8_t w_report_length;
} usb_hid_descriptor_t;

// Valid values of bRequest (See page 51 of HID1_11)
typedef enum {
    usb_hid_b_request_get_report = 0x01,
    usb_hid_b_request_get_idle = 0x02,
    usb_hid_b_request_get_protocol = 0x03,
    usb_hid_b_request_set_report = 0x09,
    usb_hid_b_request_set_idle = 0x0A,
    usb_hid_b_request_set_protocol = 0x0B
} usb_hid_b_request_t;

#endif //KEYLOGGER_HID_H
