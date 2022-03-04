//
// Created by Lorenz on 26/02/2022.
//

#ifndef KEYLOGGER_USB_HOST_HID_H
#define KEYLOGGER_USB_HOST_HID_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/usb.h"


// Function prototypes
void usb_host_hid_init_handler(void);
void usb_host_hid_report_received_handler(void);
static void usb_host_hid_set_protocol(usb_device_t * device, bool report_protocol);


#endif //KEYLOGGER_USB_HOST_HID_H
