//
// Created by Lorenz on 26/02/2022.
//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../include/drivers/usb_host_hid.h"
#include "../include/drivers/usb_hid.h"
#include "../include/hardware/usb.h"

usb_host_hid_keyboard_t hid_keyboard;
static uint8_t usb_interrupt_out_buffer[256];

/**
 * Initializes a device with an HID interface.
 * Setups interrupt in request
 *
 * @param device
 */
void usb_host_hid_init(usb_device_t * device) {
    hid_keyboard.device = device;

    // Select the boot protocol instead of the default report protocol
    usb_host_hid_set_protocol(device, false);

    // Setup Interrupt out endpoint
    struct endpoint_struct * interrupt_out_endpoint = usb_get_endpoint(1);

    usb_endpoint_init(interrupt_out_endpoint,
                      device->address,
                      device->interrupt_out_endpoint_number,
                      1,
                      device->interrupt_out_endpoint_max_packet_size,
                      usb_data_flow_types_interrupt_transfer,
                      device->interrupt_out_endpoint_polling_interval
                      );
    usb_endpoint_transfer(device->address, interrupt_out_endpoint, device->interrupt_out_endpoint_number,usb_interrupt_out_buffer,8,1);
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