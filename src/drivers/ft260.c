//
// Created by Lorenz on 2/04/2022.
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/drivers/ft260.h"
#include "../include/hardware/i2c.h"
#include "../include/hardware/gpio.h"
#include "../include/hardware/sio.h"
#include "../include/drivers/usb_hid.h"


static ft260_t *active_ft260[2];
static uint8_t report_desc[] = HID_BOOT_KEYBOARD_REPORT_DESC;
static uint16_t report_desc_size = sizeof(report_desc);


/**
 * Initializes a ft260 HID over I2C instance.
 * There can only be one active ft260 instance per i2c controller (max 2)
 *
 * @param ft260
 * @param i2c       Uninitialized i2c controller
 * @param sda_pin
 * @param scl_pin
 * @param int_pin   Interrupt pin
 */
void ft260_init(ft260_t *ft260, i2c_hw_t * i2c, uint8_t sda_pin, uint8_t scl_pin, uint8_t int_pin) {
    if (i2c == i2c0_hw) {
        active_ft260[0] = ft260;
    }
    else {
        active_ft260[1] = ft260;
    }
    
    ft260->i2c = i2c;
    ft260->int_pin = int_pin;
    ft260->output_report_available = false;
    ft260->reset_pending = false;

    // HID descriptor
    ft260->hid_descriptor.w_hid_desc_length = FT260_HID_DESC_LENGTH;
    ft260->hid_descriptor.bcd_version = FT260_HID_DESC_VERSION;
    ft260->hid_descriptor.w_input_register = ft260_input_register;
    ft260->hid_descriptor.w_output_register = ft260_output_register;
    ft260->hid_descriptor.w_command_register = ft260_command_register;
    ft260->hid_descriptor.w_data_register = ft260_data_register;
    ft260->hid_descriptor.w_report_desc_register = ft260_report_desc_register;
    ft260->hid_descriptor.w_vendor_id = FT260_VENDOR_ID;
    ft260->hid_descriptor.w_product_id = FT260_PRODUCT_ID;
    ft260->hid_descriptor.w_version_id = FT260_VERSION_ID;
    ft260->hid_descriptor.w_max_input_length = USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE;
    ft260->hid_descriptor.w_max_output_length = USB_HID_BOOT_KEYBOARD_OUTPUT_REPORT_SIZE;
    ft260->hid_descriptor.w_report_desc_length = report_desc_size;
    ft260->hid_descriptor.reserved = 0;

    // Input report queue
    ft260->input_report_queue.items = 0;
    ft260->input_report_queue.front = -1;
    ft260->input_report_queue.rear = -1;

    ft260_reset(ft260);

    // Initialize the I2C controller in slave mode
    i2c_init(i2c, 100000);
    i2c_slave_init(i2c, FT260_I2C_SLAVE_ADDRESS, ft260_i2c_handler);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_set_pullup(sda_pin, true);
    gpio_set_pullup(scl_pin, true);
    gpio_set_pulldown(sda_pin, false);
    gpio_set_pulldown(scl_pin, false);

    // Initialize the interrupt pin
    gpio_set_function(int_pin, GPIO_FUNC_SIO);
    sio_init(int_pin);
    sio_set_dir(int_pin,OUTPUT);
    ft260_de_assert_int(ft260);
}

/**
 * Asserts the interrupt line
 *
 * @param ft260
 */
static inline void ft260_assert_int(ft260_t *ft260) {
    sio_put(ft260->int_pin, 1);
}

/**
 * De-asserts the interrupt line
 *
 * @param ft260
 */
static inline void ft260_de_assert_int(ft260_t *ft260) {
    sio_put(ft260->int_pin, 0);
}

/**
 * I2C interrupt handler
 *
 * @param i2c
 * @param event
 */
void ft260_i2c_handler(i2c_hw_t *i2c, i2c_slave_event_t event) {
    // Find the ft260 instance that triggered this interrupt
    ft260_t *ft260;
    if (i2c == i2c0_hw) {
        ft260 = active_ft260[0];
    }
    else {
        ft260 = active_ft260[1];
    }

    if (event == i2c_slave_receive) {
        // Data from master available
        if (!ft260->register_selected) {
            // Register address bytes (2)
            while (i2c_is_readable(i2c) && ft260->i2c_buffer_index <= 2) {
                ft260->i2c_buffer[ft260->i2c_buffer_index++] = i2c_slave_read_byte(i2c);
            }
            if (ft260->i2c_buffer_index >= 2) {
                ft260->selected_register = ft260->i2c_buffer[1] << 8 | ft260->i2c_buffer[0];
                ft260->register_selected = true;
                ft260->i2c_buffer_index = 0;
            }
        }
        else {
            // Read the data into the buffer, we will process this data later at the end of the packet
            while (i2c_is_readable(i2c) && ft260->i2c_buffer_index < FT260_I2C_BUFFER_SIZE) {
                ft260->i2c_buffer[ft260->i2c_buffer_index++] = i2c_slave_read_byte(i2c);
            }
        }
    }
    else if (event == i2c_slave_request) {
        // Master wants to read data
        if (!ft260->register_selected) {
            // When a register address is not selected, the host is reading an input report
            if (ft260->reset_pending) {
                i2c_slave_write_byte(ft260->i2c, 0);
                i2c_slave_write_byte(ft260->i2c, 0);
                ft260->reset_pending = false;
                ft260_de_assert_int(ft260);
            }
            else {
                ft260_transfer_input_reports(ft260);
            }
        }
        else {
            // Reply to command / send the selected register
            ft260_reply_request(ft260);
        }

    }
    else if (event == i2c_slave_finish) {
        // Stop or Restart signal received form the master
        if (ft260->i2c_buffer_index != 0) {
            // There is still unprocessed data in the buffer, process it now
            ft260_reply_request(ft260);
        }
    }
}

/**
 * Send an input report to the host
 *
 * @param ft260
 * @param report
 */
void ft260_send_input_report(ft260_t *ft260, usb_hid_boot_keyboard_input_report_t *report) {
    // Enqueue the report
    if(ft260_input_report_enqueue(ft260, report)) {
        // Assert the interrupt
        ft260_assert_int(ft260);
    }

}

/**
 * Get an output report if a new one is available
 *
 * @param ft260
 * @param report Destination report
 * @return true: new report available, false: no report available
 */
bool ft260_get_output_report(ft260_t *ft260, usb_hid_boot_keyboard_output_report_t *report) {
    if (ft260->output_report_available) {
        *report = ft260->output_report;
        ft260->output_report_available = false;
        return true;
    }
    return false;
}

/**
 * Transfers the input report during the data req interrupt.
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 */
static void ft260_transfer_input_reports(ft260_t *ft260) {
    // Send the report that is in front of the queue
    if (ft260_input_report_dequeue(ft260, &ft260->registers.input.report)) {
        // Report length
        uint16_t len = USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE + 2;
        i2c_slave_write_byte(ft260->i2c, len);
        i2c_slave_write_byte(ft260->i2c, len >> 8);

        // Report
        for (int i = 0; i < USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE; i++) {
            i2c_slave_write_byte(ft260->i2c, ft260->registers.input.bytes[i]);
        }
    }

    // Check if there are more pending reports
    if (ft260_input_report_queue_is_empty(ft260)) {
        // De-assert the interrupt line
        ft260_de_assert_int(ft260);
    }
}

/**
 * Reply to a request from the host.
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 */
static void ft260_reply_request(ft260_t *ft260) {
    ft260->register_selected = false;

    // Check if there is unprocessed data in the buffer
    if (ft260->i2c_buffer_index != 0) {
        if (ft260->selected_register == ft260_command_register && ft260->i2c_buffer_index >= 2) {
            uint16_t parser_buffer_index = 0;
            // Parse the command register
            ft260_op_code_t op_code = (ft260_op_code_t)(ft260->i2c_buffer[1] & FT260_COMMAND_REGISTER_OP_CODE_BITS);
            ft260_report_type_t report_type = (ft260_report_type_t)((ft260->i2c_buffer[0] & FT260_COMMAND_REGISTER_REPORT_TYPE_BITS) >> FT260_COMMAND_REGISTER_REPORT_TYPE_LSB);
            parser_buffer_index += 2;

            // Check if there is data for in the data register
            uint16_t data_len;
            uint8_t *data;
            if (ft260->i2c_buffer_index >= 6) {
                data_len = ft260->i2c_buffer[5] << 8 | ft260->i2c_buffer[4];
                data = &ft260->i2c_buffer[6];
            }

            // Process the selected command
            switch (op_code) {
                case ft260_request_type_reset:
                    ft260_reset_cmd(ft260);
                    break;
                case ft260_request_type_get_report:
                    ft260_get_report(ft260, report_type);
                    break;
                case ft260_request_type_set_report:
                    ft260_set_report(ft260, report_type, data, data_len);
                    break;
                case ft260_request_type_set_power:
                    ft260_set_power(ft260, (ft260_power_state_t)ft260->i2c_buffer[0] & FT260_COMMAND_SET_POWER_POWER_STATE_BITS);
                    break;
                case ft260_request_type_get_protocol:
                    ft260_get_protocol(ft260);
                    break;
                case ft260_request_type_get_idle:
                    // Not supported (optional request)
                    break;
                case ft260_request_type_set_idle:
                    // Not supported (optional request)
                    break;
                case ft260_request_type_set_protocol:
                    // Not supported (optional request)
                    break;
                default:
                    break;
            }
        }
        else if (ft260->selected_register == ft260_output_register) {
            uint16_t data_len;
            uint8_t *data;
            if (ft260->i2c_buffer_index >= 3) {
                data_len = ft260->i2c_buffer[1] << 8 | ft260->i2c_buffer[0];
                data = &ft260->i2c_buffer[2];
                ft260_set_report(ft260, ft260_report_type_output, data, data_len);
            }
        }
    }
    else {
        if (ft260->selected_register == ft260_hid_desc_register) {
            // Send the hid descriptor
            for (int i = 0; i < FT260_HID_DESC_LENGTH; i++) {
                while (!i2c_is_writable(ft260->i2c));   // TODO: Remove blocking waiting loop from i2c isr
                i2c_slave_write_byte(ft260->i2c, ft260->hid_descriptor.bytes[i]);
            }
        }
        else if (ft260->selected_register == ft260_report_desc_register) {
            // Send the report descriptor
            for (int i = 0; i < report_desc_size; i++) {
                while (!i2c_is_writable(ft260->i2c));   // TODO: Remove blocking waiting loop from i2c isr
                i2c_slave_write_byte(ft260->i2c, report_desc[i]);
            }
        }
        else if (ft260->selected_register == ft260_data_register) {
            // Send the data register
            for (int i = 0; i < ft260->registers.data.length; i++) {
                while (!i2c_is_writable(ft260->i2c));   // TODO: Remove blocking waiting loop from i2c isr
                i2c_slave_write_byte(ft260->i2c, ft260->registers.data.bytes[i]);
            }
        }
    }

    ft260->i2c_buffer_index = 0;
}

/**
 * Resets the HID over I2C (ft260) driver into the initialized state
 *
 * @param ft260
 */
static void ft260_reset(ft260_t *ft260) {
    // Reset into the initialized state
    ft260->i2c_buffer_index = 0;
    ft260->power_state = ft260_power_state_on;

    // Input register

    memset(ft260->registers.input.bytes, 0, USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE);

    // Command register
    ft260->register_selected = false;
}

/**
 * Handel the reset request
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 */
static void ft260_reset_cmd(ft260_t *ft260) {
    // Reset this instance
    ft260_reset(ft260);

    // Set the reset pending flag
    ft260->reset_pending = true;

    // Asert the interrupt
    ft260_assert_int(ft260);
}

/**
 * Handle the get report request.
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 * @param report_type
 */
static void ft260_get_report(ft260_t *ft260, ft260_report_type_t report_type) {
    if (report_type == ft260_report_type_input) {
        // Fill the data register with the length of the report and the report itself

        // Calculate the length
        ft260->registers.data.length = USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE + 2;

        // Length field (2 bytes)
        ft260->registers.data.bytes[0] = ft260->registers.data.length;
        ft260->registers.data.bytes[1] = 0;

        // Report
        memcpy(&ft260->registers.data.bytes[2], &ft260->registers.input.bytes, USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE);

        // Select the data register for the next readout
        ft260->selected_register = ft260_data_register;
        ft260->register_selected = true;
    }
    else {
        // Feature reports are not supported
        // Output report is illegal
    }
}

/**
 * Handle the set report request
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 * @param report_type
 * @param data
 * @param data_len
 */
static void ft260_set_report(ft260_t *ft260, ft260_report_type_t report_type, uint8_t *data, uint16_t data_len) {
    if (report_type == ft260_report_type_output) {
        if (data_len == 3) {
            ft260->output_report = *(usb_hid_boot_keyboard_output_report_t*)data;
            ft260->output_report_available = true;
        }
    }
    else {
        // Feature reports are not supported
        // Input report is illegal
    }
}

/**
 * @brief Handle the get protocol request.
 *
 * The default protocol must be the report protocol
 * but we use the report descriptor for a boot interface for a keyboard
 * so in the rest of the application we can assume that we are using the boot protocol
 *
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 */
static void ft260_get_protocol(ft260_t *ft260) {
    // Fill the data register with the length of the report and the protocol value itself

    // Calculate the length
    ft260->registers.data.length = FT260_COMMAND_SET_PROTOCOL_PROTOCOL_VALUE_SIZE + 2;

    // Length field (2 bytes)
    ft260->registers.data.bytes[0] = ft260->registers.data.length;
    ft260->registers.data.bytes[1] = 0;

    // Protocol Value
    ft260->registers.data.bytes[2] = ft260_protocol_report;
    ft260->registers.data.bytes[3] = 0;

    ft260->selected_register = ft260_data_register;
    ft260->register_selected = true;
}

/**
 * Handle the set power request
 * @note This function is part of an interrupt handler
 *
 * @param ft260
 * @param power_state
 */
static void ft260_set_power(ft260_t *ft260, ft260_power_state_t power_state) {
    // TODO: Implement ft260 set power function
    ft260->power_state = power_state;
    switch (power_state) {
        case ft260_power_state_on:
            break;
        case ft260_power_state_sleep:
            // De-assert the interrupt line
            ft260_de_assert_int(ft260);
            break;
    }
}

/**
 * Add an report in front of the input report queue
 *
 * @param ft260
 * @param report
 * @return true: successfully added, false: failed (queue is full)
 */
static bool ft260_input_report_enqueue(ft260_t *ft260, usb_hid_boot_keyboard_input_report_t *report) {
    if (!ft260_input_report_queue_is_full(ft260)) {
        // Update front index
        if (++ft260->input_report_queue.front >= FT260_INPUT_REPORT_QUEUE_SIZE) {
            ft260->input_report_queue.front = 0;
        }
        // Add the report to the front of the queue
        ft260->input_report_queue.items++;
        ft260->input_report_queue.queue[ft260->input_report_queue.front] = *report;
        return true;
    }
    return false;
}

/**
 * Get en removes the last item from the input report queue
 *
 * @param ft260
 * @param report destination report
 * @return true: successfully dequeued, false: failed (queue is empty)
 */
static bool ft260_input_report_dequeue(ft260_t *ft260, usb_hid_boot_keyboard_input_report_t *report) {
    if (!ft260_input_report_queue_is_empty(ft260)) {
        // Update rear index
        if (++ft260->input_report_queue.rear >= FT260_INPUT_REPORT_QUEUE_SIZE) {
            ft260->input_report_queue.rear = 0;
        }
        ft260->input_report_queue.items--;
        *report = ft260->input_report_queue.queue[ft260->input_report_queue.rear];
        return true;
    }
    return false;
}

/**
 * Checks if the input report queue is full
 *
 * @param ft260
 * @return
 */
static inline bool ft260_input_report_queue_is_full(ft260_t *ft260) {
    return ft260->input_report_queue.items >= FT260_INPUT_REPORT_QUEUE_SIZE - 1;
}

/**
 * Checks if the input report queue is empty
 *
 * @param ft260
 * @return
 */
static inline bool ft260_input_report_queue_is_empty(ft260_t *ft260) {
    return ft260->input_report_queue.items <= 0;
}