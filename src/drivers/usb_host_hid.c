//
// Created by Lorenz on 26/02/2022.
//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "../include/drivers/usb_host_hid.h"
#include "../include/drivers/usb_hid.h"
#include "../include/hardware/usb.h"
#include "../include/hardware/uart.h" // TODO: remove temp header

extern usb_device_t usb_device;
static uint8_t interrupt_out_endpoint_buffer[64];

static usb_hid_boot_keyboard_input_report_t last_report;

// Report queue
static usb_hid_boot_keyboard_input_report_t report_queue[USB_HOST_HID_REPORT_QUEUE_SIZE];
int report_queue_front;
int report_queue_rear;
int report_queue_items;


/**
 * Initializes a device with an HID interface.
 * Setups interrupt in request.
 */
void usb_host_hid_init_handler(void) {
    // Init the input report queue
    usb_host_hid_init_report_queue();

    // Select the boot protocol instead of the default report protocol
    usb_host_hid_set_protocol(&usb_device, false);

    // Setup Interrupt out endpoint
    struct endpoint_struct * interrupt_out_endpoint = usb_get_endpoint(1);
    usb_device.local_interrupt_endpoint_number = 1;

    usb_endpoint_init(interrupt_out_endpoint,
                      usb_device.address,
                      usb_device.interrupt_out_endpoint_number,
                      1,
                      usb_device.interrupt_out_endpoint_max_packet_size,
                      usb_data_flow_types_interrupt_transfer,
                      usb_device.interrupt_out_endpoint_polling_interval
    );

    usb_endpoint_transfer(usb_device.address, interrupt_out_endpoint,
                          usb_device.interrupt_out_endpoint_number,interrupt_out_endpoint_buffer,
                          usb_device.interrupt_out_endpoint_max_packet_size,1);
    usb_device.hid_driver_loaded = true;
}

/**
 * Sets the protocol (boot or report protocol)
 * See 7.2.6 Set_Protocol Request of HID1_11
 *
 * @param device
 * @param report_protocol true: report protocol, false: boot protocol
 */
static void usb_host_hid_set_protocol(usb_device_t * device, bool report_protocol) {
    usb_setup_data_t setup_request = (usb_setup_data_t) {
            .bm_request_type_bits = {
                    .recipient = usb_bm_request_type_recipient_interface,
                    .type = usb_bm_request_type_type_class,
                    .direction = usb_bm_request_type_direction_host_to_dev
            },
            .b_request = usb_hid_b_request_set_protocol,
            .w_value = report_protocol ? 1 : 0,
            .w_index = device->interface_number_hid & 0xff,
            .w_length = 0
    };
    usb_send_control_transfer(device->address, &setup_request, NULL);
}

/**
 * Called when a new report is available.
 * Adds the new report to the report queue and schedules a new report
 */
void usb_host_hid_report_received_handler(void) {
    // Add report to the queue if it is different from the last one
    usb_hid_boot_keyboard_input_report_t report = *(usb_hid_boot_keyboard_input_report_t*)interrupt_out_endpoint_buffer;
    if (!usb_hid_report_cmp(&last_report, &report)) {
        usb_host_hid_report_enqueue(report);
        last_report = report;
    }

    // Schedule a new report
    struct endpoint_struct * interrupt_out_endpoint = usb_get_endpoint(1);
    usb_endpoint_transfer(usb_device.address, interrupt_out_endpoint,
                          usb_device.interrupt_out_endpoint_number,interrupt_out_endpoint_buffer,
                          usb_device.interrupt_out_endpoint_max_packet_size,1);
}

/**
 * Initializes and empties the report queue
 */
static void usb_host_hid_init_report_queue(void) {
    report_queue_items = 0;
    report_queue_front = -1;
    report_queue_rear = -1;
}

/**
 * Add a report in front of the queue
 *
 * @param report
 */
static void usb_host_hid_report_enqueue(usb_hid_boot_keyboard_input_report_t report) {
    if (!usb_host_hid_report_queue_is_full()) {
        // Update front index
        if (report_queue_front++ >= USB_HOST_HID_REPORT_QUEUE_SIZE) {
            report_queue_front = 0;
        }
        report_queue_items++;
        // Add the report to the front of the queue
        report_queue[report_queue_front] = report;
    }
    else {
        // Shouldn't happen
        // Increase the queue size if this happens
        // Discard report
    }
}

/**
 * Returns and removes the last item from the queue
 *
 * @return
 */
usb_hid_boot_keyboard_input_report_t usb_host_hid_report_dequeue(void) {
    if (!usb_host_hid_report_queue_is_empty()) {
        // Update rear index
        if (report_queue_rear++ >= USB_HOST_HID_REPORT_QUEUE_SIZE) {
            report_queue_rear = 0;
        }
        report_queue_items--;
        return report_queue[report_queue_rear];
    }
    return last_report;
}

/**
 * Checks if the report queue is full
 *
 * @return
 */
static inline bool usb_host_hid_report_queue_is_full(void) {
    return report_queue_items >= USB_HOST_HID_REPORT_QUEUE_SIZE - 1;
}

/**
 * Checks if the report queue is empty
 *
 * @return
 */
bool usb_host_hid_report_queue_is_empty(void) {
    return report_queue_items <= 0;
}

/**
 * Compares two reports on equality.
 *
 * @param report1
 * @param report2
 * @return true: reports are equal, false: not equal
 */
bool usb_hid_report_cmp(usb_hid_boot_keyboard_input_report_t *report1, usb_hid_boot_keyboard_input_report_t *report2) {
    // Check modifier keys
    if (report1->modifier_keys != report2->modifier_keys) {
        return false;
    }

    // Check keycodes
    for (int i = 0; i < 6; i++) {
        // TODO: The order of keycodes can technically change between reports
        if (report1->keycodes[i] != report2->keycodes[i]) {
            return false;
        }
    }

    return true;
}

/**
 * Initializes a report parser instance
 *
 * @return
 */
usb_hid_keyboard_report_parser_t hid_report_parser_init(void) {
    usb_hid_keyboard_report_parser_t parser;

    // TODO: Add option to select a keymap

    // Initialise struct members
    parser.prev_report = (usb_hid_boot_keyboard_input_report_t){0, 0, 0, 0, 0, 0, 0, 0};
    parser.pressed_modifiers = (usb_hid_modifier_keys_bits_t){0, 0, 0, 0, 0, 0, 0, 0};
    memset(parser.pressed_keys, 0, 6);
    parser.capslock_enabled = false;

    return parser;
}

/**
 * Parse a hid boot keyboard input report, converts the keycodes to ASCII
 *
 * @param parser
 * @param report
 * @param pressed_keys 6 byte destination array where the pressed keys should be stored
 * @return
 */
int hid_report_parse(usb_hid_keyboard_report_parser_t *parser, usb_hid_boot_keyboard_input_report_t *report, uint8_t *pressed_keys) {
    bool key_in_prev_report = false;
    int pressed_keys_count = 0;
    int layer = 0; // 0 = default, 1 = shift, 2 right alt

    // Modifier keys
    parser->pressed_modifiers = report->modifier_keys_bits;
    if((parser->pressed_modifiers.left_shift || parser->pressed_modifiers.right_shift) ^ parser->capslock_enabled) {
        layer = 1;
    }
    else if(parser->pressed_modifiers.right_alt) {
        layer = 2;
    }

    // Keycodes
    for (int i = 0; i < 6; i++) {
        // Check if the key is already pressed
        key_in_prev_report = false;
        for (int j = 0; j < 6; j++) {
            if (report->keycodes[i] == parser->pressed_keys[j]){
                key_in_prev_report = true;
                break;
            }
        }
        if (!key_in_prev_report) {
            // New key pressed, parse
            uint8_t ascii_code = usb_hid_keycode_to_ascii_qwerty[report->keycodes[i]][layer];
            if (ascii_code != 0) {
                // Key with an ascii code relationship, add to the pressed keys array
                pressed_keys[pressed_keys_count++] = ascii_code;
            }
            else if (report->keycodes[i] == USB_HID_KEY_CAPS_LOCK) {
                parser->capslock_enabled = !parser->capslock_enabled;
            }
        }
    }

    return pressed_keys_count;
}