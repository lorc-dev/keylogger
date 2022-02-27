//
// Created by Lorenz on 26/02/2022.
//

#ifndef KEYLOGGER_USB_HOST_HID_H
#define KEYLOGGER_USB_HOST_HID_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/usb.h"

typedef struct {
    usb_device_t * device;
    struct endpoint_struct interrupt_out_endpoint;
} usb_host_hid_keyboard_t;


// Function prototypes
void usb_host_hid_init(usb_device_t * device);
static void usb_host_hid_set_protocol(usb_device_t * device, bool report_protocol);

#endif //KEYLOGGER_USB_HOST_HID_H
