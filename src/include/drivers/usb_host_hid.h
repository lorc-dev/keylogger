//
// Created by Lorenz on 26/02/2022.
//

#ifndef KEYLOGGER_USB_HOST_HID_H
#define KEYLOGGER_USB_HOST_HID_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/usb.h"
#include "../drivers/usb_hid.h"

#define USB_HOST_HID_REPORT_QUEUE_SIZE  10


// Function prototypes
void usb_host_hid_init_handler(void);
void usb_host_hid_report_received_handler(void);
void usb_host_hid_send_output_report(usb_hid_boot_keyboard_output_report_t *report);
static void usb_host_hid_set_protocol(usb_device_t * device, bool report_protocol);
static void usb_host_set_report(usb_hid_report_type_t report_type, uint8_t *report, uint16_t report_len);
static void usb_host_hid_init_report_queue(void);
static void usb_host_hid_report_enqueue(usb_hid_boot_keyboard_input_report_t report);
usb_hid_boot_keyboard_input_report_t usb_host_hid_report_dequeue(void);
static inline bool usb_host_hid_report_queue_is_full(void);
bool usb_host_hid_report_queue_is_empty(void);

#endif //KEYLOGGER_USB_HOST_HID_H
