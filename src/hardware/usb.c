//
// Created by Lorenz on 20/02/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/lib/math.h"
#include "../include/hardware/usb.h"
#include "../include/hardware/resets.h"
#include "../include/hardware/irq.h"
#include "../include/hardware/timer.h"
#include "../include/hardware/uart.h"   // TODO: remove temp import
#include "../include/drivers/usb_host_hid.h"
#include "../include/events/events.h"

struct endpoint_struct endpoints[16];
static uint8_t usb_ctrl_buffer[64];
usb_device_t *usb_device;    // Only one usb device


/**
 * Initializes the USB controller in host mode
 */
void usb_init(usb_device_t *device) {
    usb_device = device;
    // Init usb_device struct
    usb_device->address = 0;       // Standard address
    usb_device->max_packet_size_ep_0 = 64;
    usb_device->connected = false;
    usb_device->enumerated = false;
    usb_device->hid_driver_loaded = false;

    // Reset USB controller
    reset_subsystem(RESETS_RESET_USBCTRL);
    unreset_subsystem_wait(RESETS_RESET_USBCTRL);

    // Clear dpsram
    memset(usb_host_dpsram, 0, sizeof(*usb_host_dpsram));

    // Init endpoints
    usb_init_endpoints();

    // Connect the controller to the USB PHY
    usb_hw->usb_muxing = USB_MUXING_TO_PHY_BIT | USB_MUXING_SOFTCON_BIT;

    // Force VBUS detect
    usb_hw->usb_pwr = USB_PWR_VBUS_DETECT_BIT | USB_PWR_VBUS_DETECT_OVERRIDE_EN_BIT;

    // Setup irq handler
    //irq_set_handler(USBCTRL_IRQ, usb_irq);

    // Enable host mode
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BIT | USB_MAIN_CTRL_HOST_NDEVICE_BIT;
    usb_hw->sie_ctrl =  USB_SIE_CTRL_KEEP_ALIVE_EN_BIT |
                        USB_SIE_CTRL_SOF_EN_BIT |
                        USB_SIE_CTRL_PULLDOWN_EN_BIT |
                        USB_SIE_CTRL_EP0_INT_1BUF_BIT;
    usb_hw->inte = USB_INTE_BUFF_STATUS_BIT      |
                   USB_INTE_HOST_CONN_DIS_BIT    |
                   USB_INTE_HOST_RESUME_BIT      |
                   USB_INTE_STALL_BIT            |
                   USB_INTE_TRANS_COMPLETE_BIT   |
                   USB_INTE_ERROR_RX_TIMEOUT_BIT |
                   USB_INTE_ERROR_DATA_SEQ_BIT   ;

    // Enable USB_CTRL Interrupt request
    irq_set_enabled(USBCTRL_IRQ, true);
}

static void usb_init_endpoints(void) {
    // Endpoint 0 (epx)
    endpoints[0].buffer_control = &usb_host_dpsram->epx_buff_ctrl;
    endpoints[0].endpoint_control = &usb_host_dpsram->epx_ctrl;
    endpoints[0].dps_data_buffer = &usb_host_dpsram->data_buffers[0];
    endpoints[0].w_max_packet_size = usb_device->max_packet_size_ep_0;
    endpoints[0].interrupt_number = 0;
    endpoints[0].interrupt_interval = 0;

    // Interrupt endpoints
    for (int i = 1; i < 16; i++) {
        endpoints[i].buffer_control = &usb_host_dpsram->int_ep_buff_ctrl[i - 1].ctrl;
        endpoints[i].endpoint_control = &usb_host_dpsram->int_ep_ctrl[i - 1].ctrl;
        endpoints[i].dps_data_buffer = &usb_host_dpsram->data_buffers[64 * (i)];
        endpoints[i].interrupt_number = i;
    }
}

/**
 * USB IRQ handler
 */
void usb_irq(void) {
    uint32_t status = usb_hw->ints;

    // Device connected or disconnected
    if (status & USB_INTS_HOST_CONN_DIS_BIT) {
        if (device_speed()) {
            // USB device connected
            event_add(event_usb_device_attached, usb_device_attach_handler);
        }
        else {
            // USB device disconnected
            event_add(event_usb_device_detached, usb_device_detach_handler);
        }
        // Clear interrupt
        usb_hw->sie_status = USB_SIE_STATUS_SPEED_BITS;
    }

    if (status & USB_INTS_TRANS_COMPLETE_BIT)
    {
        handle_transfer_complete();
        usb_hw->sie_status = USB_SIE_STATUS_TRANS_COMPLETE_BIT;
    }

    if (status & USB_INTS_BUFF_STATUS_BIT)
    {
        usb_handle_buff_status();
        //uart_puts(uart0_hw, "buff");
    }

    if (status & USB_INTS_STALL_BIT)
    {
        // TODO: handle STALL irq
        uart_puts(uart0_hw, "stall");
        usb_hw->sie_status = USB_SIE_STATUS_STALL_REC_BIT;
    }

    if (status & USB_INTS_ERROR_RX_TIMEOUT_BIT)
    {
        // TODO: handle Timeout IRQ
        uart_puts(uart0_hw, "rx_timeout");
        usb_hw->sie_status = USB_SIE_STATUS_RX_TIMEOUT_BIT;
    }

    if (status & USB_INTS_ERROR_DATA_SEQ_BIT)
    {
        // TODO: handle data seq error
        uart_puts(uart0_hw, "data_seq");
        usb_hw->sie_status = USB_SIE_STATUS_DATA_SEQ_ERROR_BIT;
    }
}

extern void isr_irq5(void) {
    usb_irq();
}

/**
 * Device speed. Disconnected = 0, LS= 1, FS = 2
 *
 * @return Device speed
 */
static inline dev_speed_t device_speed(void) {
    return (usb_hw->sie_status & USB_SIE_STATUS_SPEED_BITS) >> USB_SIE_STATUS_SPEED_LSB;
}

/**
 * USB device attached event
 *
 * @param device
 */
void usb_device_attach_handler(void) {
    usb_device->connected = true;
    usb_enum_device(usb_device);
}

/**
 * USB device detached event
 *
 * @param device
 */
void usb_device_detach_handler(void) {
    usb_device->address = 0;       // Standard address
    usb_device->max_packet_size_ep_0 = 64;
    usb_device->connected = false;
    usb_device->enumerated = false;
    usb_device->hid_driver_loaded = false;
}

/**
 * Enumerate and configure the USB device
 *
 * @param device
 */
void usb_enum_device(usb_device_t * device) {
    device->speed = device_speed();

    // Get first 8 bytes of device descriptor to get endpoint 0 size
    usb_setup_data_t setup_request = (usb_setup_data_t) {
        .bm_request_type_bits = {
                .recipient = usb_bm_request_type_recipient_device,
                .type = usb_bm_request_type_type_standard,
                .direction = usb_bm_request_type_direction_dev_to_host
        },
        .b_request = usb_setup_req_b_req_type_get_descriptor,
        .w_value = usb_descriptor_types_device << 8,
        .w_index = 0,
        .w_length = 8
    };
    usb_send_control_transfer(0, &setup_request, usb_ctrl_buffer);
    device->max_packet_size_ep_0 = ((usb_device_descriptor_t *) usb_ctrl_buffer)->b_max_packet_size_0;
    endpoints[0].w_max_packet_size = device->max_packet_size_ep_0;  // Only one device, so set max_packet_size in endpoint struct

    // Set new address
    setup_request = (usb_setup_data_t) {
            .bm_request_type_bits = {
                    .recipient = usb_bm_request_type_recipient_device,
                    .type = usb_bm_request_type_type_standard,
                    .direction = usb_bm_request_type_direction_host_to_dev
            },
            .b_request = usb_setup_req_b_req_type_set_address,
            .w_value = USB_DEVICE_ADDRESS,
            .w_index = 0,
            .w_length = 0
    };
    usb_send_control_transfer(0, &setup_request, NULL);
    device->address = USB_DEVICE_ADDRESS;
    wait_ms(2); // Device has 2 ms to get ready to respond to the new address

    // Get full device descriptor
    setup_request = (usb_setup_data_t) {
            .bm_request_type_bits = {
                    .recipient = usb_bm_request_type_recipient_device,
                    .type = usb_bm_request_type_type_standard,
                    .direction = usb_bm_request_type_direction_dev_to_host
            },
            .b_request = usb_setup_req_b_req_type_get_descriptor,
            .w_value = usb_descriptor_types_device << 8,
            .w_index = 0,
            .w_length = 18
    };
    usb_send_control_transfer(device->address, &setup_request, usb_ctrl_buffer);
    // Extract data from the descriptor
    device->product_id = ((usb_device_descriptor_t *) usb_ctrl_buffer)->id_product;
    device->vendor_id = ((usb_device_descriptor_t *) usb_ctrl_buffer)->id_vendor;
    device->configuration_count = ((usb_device_descriptor_t *) usb_ctrl_buffer)->b_num_configurations;

    // Get first 9 bytes of configuration descriptor
    setup_request = (usb_setup_data_t) {
            .bm_request_type_bits = {
                    .recipient = usb_bm_request_type_recipient_device,
                    .type = usb_bm_request_type_type_standard,
                    .direction = usb_bm_request_type_direction_dev_to_host
            },
            .b_request = usb_setup_req_b_req_type_get_descriptor,
            .w_value = (usb_descriptor_types_configuration << 8) | (USB_CONFIGURATION_NUMBER - 1),
            .w_index = 0,
            .w_length = 9
    };
    usb_send_control_transfer(device->address, &setup_request, usb_ctrl_buffer);

    // Get full config descriptor (+ interface, endpoint,... descriptor)
    setup_request.w_length = ((usb_configuration_descriptor_t *) usb_ctrl_buffer)->w_total_length;  // Update the request length
    usb_send_control_transfer(device->address, &setup_request, usb_ctrl_buffer);
    device->interface_count = ((usb_configuration_descriptor_t *) usb_ctrl_buffer)->b_num_interfaces;


    // Set configured (Select first configuration)
    setup_request = (usb_setup_data_t) {
            .bm_request_type_bits = {
                    .recipient = usb_bm_request_type_recipient_device,
                    .type = usb_bm_request_type_type_standard,
                    .direction = usb_bm_request_type_direction_host_to_dev
            },
            .b_request = usb_setup_req_b_req_type_set_configuration,
            .w_value = USB_CONFIGURATION_NUMBER,
            .w_index = 0,
            .w_length = 0
    };
    usb_send_control_transfer(device->address, &setup_request, NULL);

    // Parse configuration and interfaces to find the HID interface
    bool supported_interface_found = false;
    bool interrupt_out_endpoint_found = false;
    uint8_t * parser_ptr = usb_ctrl_buffer + 9; // Configuration descriptor is 9 bytes TODO: move to define
    while (parser_ptr < usb_ctrl_buffer + ((usb_configuration_descriptor_t *)usb_ctrl_buffer)->w_total_length) {
        // Check whether it is an interface descriptor, endpoint descriptor or another descriptor
        if (parser_ptr[USB_DESCRIPTOR_B_DESCRIPTOR_TYPE_OFFSET] == usb_descriptor_types_interface && !supported_interface_found) {
            // It is an interface descriptor
            usb_interface_descriptor_t * interface_descriptor = (usb_interface_descriptor_t *) parser_ptr;

            // Check if the class is HID or not
            if (interface_descriptor->b_interface_class == USB_CLASS_CODE_HID) {
                // HID class
                device->interface_number_hid = interface_descriptor->b_interface_number;
                supported_interface_found = true;
            }
        }
        else if (parser_ptr[USB_DESCRIPTOR_B_DESCRIPTOR_TYPE_OFFSET] == usb_descriptor_types_endpoint && !interrupt_out_endpoint_found) {
            // It is an endpoint descriptor
            usb_endpoint_descriptor_t * endpoint_descriptor = (usb_endpoint_descriptor_t *) parser_ptr;
            // Check if it is an Interrupt IN endpoint
            if (endpoint_descriptor->bm_attributes_bits.transfer_type == usb_data_flow_types_interrupt_transfer && endpoint_descriptor->b_endpoint_address_bits.direction == 1) {
                device->interrupt_out_endpoint_number = endpoint_descriptor->b_endpoint_address_bits.endpoint_number;
                device->interrupt_out_endpoint_polling_interval = endpoint_descriptor->b_interval;
                device->interrupt_out_endpoint_max_packet_size = endpoint_descriptor->w_max_packet_size & USB_ENDPOINT_DESCRIPTOR_MAX_PACKET_SIZE_BITS;
                interrupt_out_endpoint_found = true;
            }
        }

        if(interrupt_out_endpoint_found && supported_interface_found) {
            // Stop parsing
            break;
        }
        else {
            // Go to the next descriptor
            parser_ptr += parser_ptr[USB_DESCRIPTOR_B_LENGTH_OFFSET]; // Increase by the descriptors length
        }
    }

    if (supported_interface_found && interrupt_out_endpoint_found) {
        // Load HID driver
        event_add(event_usb_host_hid_load_driver, usb_host_hid_init_handler);
    }
    else {
        // TODO: error: device is not hid?
    }
    device->enumerated = true;
}

/**
 * Resets the usb bus
 */
static void usb_reset_bus(void) {
    usb_hw->sie_ctrl |= USB_SIE_CTRL_RESET_BUS_BIT;
}

/**
 * Send a setup packet with the given data
 *
 * @param device_address
 * @param setup_packet
 */
static void usb_setup_send(uint8_t device_address, usb_setup_data_t *setup_packet) {

    // Copy the packet into the setup packet buffer
    memcpy((void*)usb_host_dpsram->setup_packet, setup_packet, 8);

    // Configure endpoint 0 with setup info
    struct endpoint_struct * endpoint = &endpoints[0];
    usb_endpoint_init(endpoint, device_address, 0, 0, endpoint->w_max_packet_size, usb_data_flow_types_control_transfer, 0);
    endpoint->total_len = 8;
    endpoint->transfer_size = 8;
    endpoint->active = true;
    endpoint->setup = true;

    // Set device address
    usb_hw->dev_addr_ep_ctrl = device_address;

    // Send setup
    usb_hw->sie_ctrl =  USB_SIE_CTRL_KEEP_ALIVE_EN_BIT |
                        USB_SIE_CTRL_SOF_EN_BIT |
                        USB_SIE_CTRL_PULLDOWN_EN_BIT |
                        USB_SIE_CTRL_EP0_INT_1BUF_BIT |
                        USB_SIE_CTRL_SEND_SETUP_BIT |
                        USB_SIE_CTRL_START_TRANS_BIT;
}

/**
 * Send a control transfer
 *
 * @param device_address
 * @param setup_packet
 * @param data
 */
void usb_send_control_transfer(uint8_t device_address, usb_setup_data_t *setup_packet, uint8_t * data) {
    // Use endpoint 0 for control transfers
    struct endpoint_struct * endpoint = &endpoints[0];

    // Send setup packet
    usb_setup_send(device_address, setup_packet);
    while(endpoint->active);

    // Data stage
    if (setup_packet->w_length) {
        usb_endpoint_transfer(device_address, endpoint, 0, data, setup_packet->w_length, setup_packet->bm_request_type_bits.direction);
        while(endpoint->active);
    }

    // Status stage
    usb_endpoint_transfer(device_address, endpoint, 0, NULL, 0, 1 - setup_packet->bm_request_type_bits.direction);
    while(endpoint->active);

    // TODO: Check if failed or stalled
}


/**
 * Start a transaction
 *
 * @param device_address
 * @param endpoint
 * @param endpoint_number
 * @param buffer
 * @param buffer_len
 * @param direction
 */
void usb_endpoint_transfer(uint8_t device_address, struct endpoint_struct *endpoint, uint16_t endpoint_number,uint8_t * buffer, uint16_t buffer_len, uint8_t direction) {
   // Endpoint init
   if (endpoint->device_address != device_address || direction != endpoint->receive) {
       usb_endpoint_init(endpoint, device_address, endpoint_number, direction, endpoint->w_max_packet_size,
                         endpoint->transfer_type, endpoint->interrupt_interval);
   }

   endpoint->active = true;
   endpoint->transferred_bytes = 0;
   endpoint->total_len = buffer_len;
   endpoint->transfer_size = min(buffer_len, max(endpoint->w_max_packet_size, 64));
   endpoint->mem_data_buffer = buffer;

   // Check if it is the last buffer
   endpoint->last_buffer = ((endpoint->transferred_bytes + endpoint->transfer_size) >= endpoint->total_len);
   endpoint->buffer_selector = 0;

    usb_endpoint_transfer_buffer(endpoint);

    // If endpoint 0 (non interrupt)
    if (endpoint->interrupt_number == 0){
        usb_hw->dev_addr_ep_ctrl = device_address | (endpoint_number << USB_ADDR_ENDP_ENDPOINT_LSB);
        usb_hw->sie_ctrl = USB_SIE_CTRL_KEEP_ALIVE_EN_BIT |
                           USB_SIE_CTRL_SOF_EN_BIT |
                           USB_SIE_CTRL_PULLDOWN_EN_BIT |
                           USB_SIE_CTRL_EP0_INT_1BUF_BIT |
                           USB_SIE_CTRL_START_TRANS_BIT |
                           (direction ? USB_SIE_CTRL_RECEIVE_DATA_BIT : USB_SIE_CTRL_SEND_DATA_BIT);
    }
}

/**
 * Setup the buffer control register
 *
 * @param endpoint
 */
void usb_endpoint_transfer_buffer(struct endpoint_struct * endpoint) {
    uint32_t buffer_control_val = endpoint->transfer_size;

    if (endpoint->receive == false) {
        // Copy data from the temp buffer in mem to the hardware buffer
        memcpy(endpoint->dps_data_buffer, &endpoint->mem_data_buffer[endpoint->transferred_bytes], endpoint->transfer_size);
        buffer_control_val |= USB_BUFF_CTRL_BUFF0_FULL_BIT;
    }

    // Set PID (DATA0 or DATA1)
    buffer_control_val |= endpoint->pid ? USB_BUFF_CTRL_BUFF0_DATA_PID_BIT : 0;

    // Set the PID for the next transfer
    if (endpoint->transfer_size == 0) {
        endpoint->pid ^= 1u;
    }
    else {
        uint32_t packet_count = 1 + ((endpoint->transfer_size - 1) / endpoint->w_max_packet_size);
        if (packet_count & 0x01) {
            endpoint->pid ^= 1u;
        }
    }

    // Check if it is the last buffer, if so, set th last buffer bit
    if (endpoint->last_buffer)
        buffer_control_val |= USB_BUFF_CTRL_BUFF0_LAST_BIT;

    // Copy the value to the buffer control register, this will start the transfer
    *(endpoint->buffer_control) = buffer_control_val;
    // Set available bit when the other bits are already set and stable
    *(endpoint->buffer_control) |= USB_BUFF_CTRL_AVAILABLE_0_BIT;
}

/**
 * Init an endpoint with the given data
 *
 * @param endpoint
 * @param device_address
 * @param endpoint_number
 * @param direction 1 = RX / IN, 0 = TX / OUT
 * @param w_max_packet_size
 * @param transfer_type
 * @param b_interval
 */
void usb_endpoint_init(struct endpoint_struct *endpoint, uint8_t device_address, uint8_t endpoint_number, uint8_t direction, uint16_t w_max_packet_size, usb_data_flow_types_t transfer_type, uint8_t b_interval) {
    endpoint->device_address = device_address;
    endpoint->endpoint_number = endpoint_number;
    endpoint->interrupt_interval = b_interval;

    endpoint->receive = direction;

    endpoint->pid = (endpoint_number == 0) ? 1u : 0u;
    endpoint->w_max_packet_size = w_max_packet_size;
    endpoint->transfer_type = transfer_type;


    uint32_t dpsram_offset = (uintptr_t)((uintptr_t)endpoint->dps_data_buffer ^ (uintptr_t)usb_host_dpsram);

    *(endpoint->endpoint_control) = USB_EP_CTRL_ENABLE_BIT |
                                    USB_EP_CTRL_INT_BUFFER_BIT |
                                    (transfer_type << USB_EP_CTRL_TYPE_LSB) |
                                    (dpsram_offset) |
                                    (b_interval ? (b_interval - 1) << USB_EP_CTRL_HOST_INT_INTERVAL_LSB : 0);

    if (b_interval) {
        // This is an interrupt endpoint
        // Set up interrupt endpoint address control register:
        usb_hw->host_int_ep_addr_ctrl[endpoint->interrupt_number - 1] = device_address | (endpoint_number << USB_ADDR_ENDPN_ENDPOINT_LSB);

        // Enable the interrupt endpoint
        usb_hw->int_ep_ctrl = 1 << endpoint->interrupt_number;
    }
}

/**
 * Handle buff_status irq for each endpoint (0-15)
 */
static void usb_handle_buff_status(void) {
    uint32_t completed_buffers = usb_hw->buff_status;

    for (int i = 0; i < 16; i++) {
        uint32_t bit = 1ul << (i*2);
        if (completed_buffers & bit) {
            // Clear the bit (write to clear register)
            usb_hw->buff_status = bit;
            bool done = usb_endpoint_transfer_continue(&endpoints[i]);
            if (done) {
                usb_endpoint_reset(&endpoints[i]);

                // Check if this is for the HID driver
                if (usb_device->hid_driver_loaded && endpoints[i].interrupt_number == usb_device->local_interrupt_endpoint_number) {
                    // Call hid report handler
                    event_add(event_usb_host_hid_report_available, usb_host_hid_report_received_handler);
                }
            }
        }
    }
}

/**
 * Checks if a transaction is complete and prepare for a potential next
 *
 * @param endpoint
 * @return true: transaction is complete, false: not done yet
 */
static bool usb_endpoint_transfer_continue(struct endpoint_struct *endpoint) {
    // Get hardware buffer state and extract the amount of transferred bytes
    uint32_t buffer_ctrl = *(endpoint->buffer_control);

    // RP2040-E4 bug
    // Summary:     USB host writes to upper half of buffer status in single buffered mode
    // Workaround:  Shift endpoint control register to the right by 16 bits if the buffer selector is BUF1. You can use
    //              BUFF_CPU_SHOULD_HANDLE find the value of the buffer selector when the buffer was marked as done.
     if (endpoint->buffer_selector == 1) {
        buffer_ctrl = buffer_ctrl >> 16;
        *(endpoint->buffer_control) = buffer_ctrl;
    }
    endpoint->buffer_selector ^= 1ul;   // Flip buffer selector

    uint16_t transferred_bytes = buffer_ctrl & USB_BUFF_CTRL_BUFF0_TRANSFER_LENGTH_BITS;


    // Update the bytes sent or received
    if (endpoint->receive == false) {
        // Update bytes sent
        endpoint->transferred_bytes += transferred_bytes;
    }
    else {
        // Copy received data to the memory buffer at the correct index
        memcpy(&endpoint->mem_data_buffer[endpoint->transferred_bytes], endpoint->dps_data_buffer, transferred_bytes);
        endpoint->transferred_bytes += transferred_bytes;
    }


    // Check if less data is sent than the transfer size (=> last packet)
    if (endpoint->receive && (transferred_bytes < endpoint->transfer_size)) {
        // Update the total length
        endpoint->total_len = endpoint->transferred_bytes;
    }

    uint16_t remaining_bytes = endpoint->total_len - endpoint->transferred_bytes;
    endpoint->transfer_size = min(remaining_bytes, max(endpoint->w_max_packet_size, 64));
    endpoint->last_buffer = ((endpoint->transferred_bytes + endpoint->transfer_size) >= endpoint->total_len);

    // Done
    if (remaining_bytes == 0) {
        return true;
    }
    else {
        usb_endpoint_transfer_buffer(endpoint);
    }

    // Not done yet
    return false;
}

/**
 * Resets the given endpoint
 *
 * @param endpoint
 */
static void usb_endpoint_reset(struct endpoint_struct *endpoint) {
    // Reset endpoint
    endpoint->total_len = 0;
    endpoint->transferred_bytes = 0;
    endpoint->transfer_size = 0;
    endpoint->mem_data_buffer = 0;
    endpoint->active = false;
    endpoint->setup = false;
}

/**
 * Handle transfer_complete IRQ
 */
static void handle_transfer_complete(void) {
    struct endpoint_struct * endpoint = &endpoints[0];
    if (endpoint->setup) {
        usb_endpoint_reset(endpoint);
    }
}

/**
 * Returns the endpoint_struct pointer to the selected endpoint
 *
 * @param endpoint_number
 * @return
 */
struct endpoint_struct * usb_get_endpoint(uint8_t endpoint_number) {
    return (struct endpoint_struct *)&endpoints[endpoint_number];
}