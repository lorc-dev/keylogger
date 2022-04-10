//
// Created by Lorenz on 2/04/2022.
//

#ifndef KEYLOGGER_FT260_H
#define KEYLOGGER_FT260_H

#include <stdint.h>
#include <stdbool.h>
#include "../hardware/i2c.h"
#include "../drivers/usb_hid.h"


#define FT260_I2C_SLAVE_ADDRESS 0x10
#define FT260_I2C_BUFFER_SIZE   16  // Size in bytes (max 255)l

#define FT260_VENDOR_ID     0x0403  // Default from FTDI
#define FT260_PRODUCT_ID    0x6030  // Default from FTDI
#define FT260_VERSION_ID    0x01

// Input report queue
#define FT260_INPUT_REPORT_QUEUE_SIZE   5

// HID descriptor
#define FT260_HID_DESC_LENGTH   0x1E
#define FT260_HID_DESC_VERSION  0x100

// Data register
#define FT260_DATA_REGISTER_SIZE    16

// Command register (See 7.1.1 Command Register)
#define FT260_COMMAND_REGISTER_OP_CODE_BITS     0xFul
#define FT260_COMMAND_REGISTER_REPORT_TYPE_BITS (0x3 << 4)
#define FT260_COMMAND_REGISTER_REPORT_TYPE_LSB  4

// Set power command (See 7.2.8 SET_POWER)
#define FT260_COMMAND_SET_POWER_POWER_STATE_BITS    0x03

// Get protocol command (See 7.2.6 GET_PROTOCOL)
#define FT260_COMMAND_SET_PROTOCOL_PROTOCOL_VALUE_SIZE  2

// I2C registers
typedef enum {
    ft260_hid_desc_register = 1,
    ft260_input_register,
    ft260_report_desc_register,
    ft260_output_register,
    ft260_command_register,
    ft260_data_register,
} ft260_i2c_registers_t;

//// Reference: HID over I2C protocol specification

// Report types (See page 29: Report Type)
typedef enum {
    ft260_report_type_input = 1,
    ft260_report_type_output,
    ft260_report_type_feature
} ft260_report_type_t;

// Op codes (See 7.2 Class Specific Requests)
typedef enum {
    ft260_request_type_reset = 1,           // Reset the device at any time
    ft260_request_type_get_report,      // Request from HOST to DEVICE to retrieve a report (input/feature)
    ft260_request_type_set_report,      // Request from HOST to DEVICE to set a report (input/feature)
    ft260_request_type_get_idle,        // Request from HOST to DEVICE to retrieve the current idle rate for a particular TLC
    ft260_request_type_set_idle,        // Request from HOST to DEVICE to set the current idle rate for a particular TLC
    ft260_request_type_get_protocol,    // Request from HOST to DEVICE to retrieve the protocol mode the device is operating in
    ft260_request_type_set_protocol,    // Request from HOST to DEVICE to set the protocol mode the device should be operating in
    ft260_request_type_set_power,       // Request from HOST to DEVICE to indicate preferred power setting
} ft260_op_code_t;

// Power states (See 8.2 Host Initiated Power Optimizations)
typedef enum {
    ft260_power_state_on = 0,
    ft260_power_state_sleep,
} ft260_power_state_t;

// Protocol values (See 7.2.6 GET_PROTOCOL)
typedef enum {
    ft260_protocol_boot = 0,
    ft260_protocol_report,
} ft260_protocol_t;

// HID descriptor format (See Table 1: HID Descriptor Layout)
typedef union {
        struct {
            uint16_t w_hid_desc_length;         // The length, in unsigned bytes, og the complete HID descriptor
            uint16_t bcd_version;               // The version number, in binary coded decimal (BCD) format. DEVICE should default to 0x0100
            uint16_t w_report_desc_length;      // The length, in unsigned bytes, of the Report Descriptor.
            uint16_t w_report_desc_register;    // The register index containing the Report Descriptor on the DEVICE
            uint16_t w_input_register;          // This field identifies, in unsigned bytes, the register number to read the input report from the DEVICE
            uint16_t w_max_input_length;        // This field identifies in unsigned bytes the length of the largest Input Report to be read from the Input Register (Complex HID Devices will need various sized reports).
            uint16_t w_output_register;         // This field identifies, in unsigned bytes, the register number to send the output report to the DEVICE
            uint16_t w_max_output_length;       // This field identifies in unsigned bytes the length of the largest output Report to be sent to the Output Register (Complex HID Devices will need various sized reports).
            uint16_t w_command_register;        // This field identifies, in unsigned bytes, the register number to send command requests to the DEVICE
            uint16_t w_data_register;           // This field identifies in unsigned bytes the register number to exchange data with the Command Request
            uint16_t w_vendor_id;               // This field identifies the DEVICE manufacturers Vendor ID. Must be non-zero.
            uint16_t w_product_id;              // This field identifies the DEVICE’s unique model / Product ID.
            uint16_t w_version_id;              // This field identifies the DEVICE’s unique model / Product ID.
            uint32_t reserved;                  // This field identifies the DEVICE’s unique model / Product ID.
        };
        uint8_t bytes[30];
    } ft260_hid_descriptor_t;

typedef struct {
    usb_hid_boot_keyboard_input_report_t queue[FT260_INPUT_REPORT_QUEUE_SIZE];
    int front;
    int rear;
    unsigned int items;
} ft260_input_report_queue_t;

typedef struct {
    // Hardware
    i2c_hw_t *i2c;
    uint8_t int_pin;

    ft260_hid_descriptor_t hid_descriptor;
    ft260_input_report_queue_t input_report_queue;
    usb_hid_boot_keyboard_output_report_t output_report;
    bool output_report_available;
    bool reset_pending;

    // Buffer
    uint8_t i2c_buffer[FT260_I2C_BUFFER_SIZE];
    uint8_t i2c_buffer_index;

    // Registers
    uint16_t selected_register;
    bool register_selected;
    struct {
        // Data register
        struct {
            union {
                struct {
                    uint16_t len;
                    uint8_t data[FT260_DATA_REGISTER_SIZE - 2];
                };
                uint8_t bytes[FT260_DATA_REGISTER_SIZE];
            };
            uint8_t length;
        } data;

        // Input register
        union {
            usb_hid_boot_keyboard_input_report_t report;
            uint8_t bytes[USB_HID_BOOT_KEYBOARD_INPUT_REPORT_SIZE];
        } input;
    } registers;

    ft260_power_state_t power_state;
} ft260_t;


// Function prototypes
void ft260_init(ft260_t *ft260, i2c_hw_t * i2c, uint8_t sda_pin, uint8_t scl_pin, uint8_t int_pin);
static inline void ft260_assert_int(ft260_t *ft260);
static inline void ft260_de_assert_int(ft260_t *ft260);
void ft260_i2c_handler(i2c_hw_t *i2c, i2c_slave_event_t event);
void ft260_send_input_report(ft260_t *ft260, usb_hid_boot_keyboard_input_report_t *report);
bool ft260_get_output_report(ft260_t *ft260, usb_hid_boot_keyboard_output_report_t *report);
static void ft260_transfer_input_reports(ft260_t *ft260);
static void ft260_reply_request(ft260_t *ft260);
static void ft260_reset(ft260_t *ft260);
static void ft260_reset_cmd(ft260_t *ft260);
static void ft260_get_report(ft260_t *ft260, ft260_report_type_t report_type);
static void ft260_set_report(ft260_t *ft260, ft260_report_type_t report_type, uint8_t *data, uint16_t data_len);
static void ft260_get_protocol(ft260_t *ft260);
static void ft260_set_power(ft260_t *ft260, ft260_power_state_t power_state);
static bool ft260_input_report_enqueue(ft260_t *ft260, usb_hid_boot_keyboard_input_report_t *report);
static bool ft260_input_report_dequeue(ft260_t *ft260, usb_hid_boot_keyboard_input_report_t *report);
static inline bool ft260_input_report_queue_is_full(ft260_t *ft260);
static inline bool ft260_input_report_queue_is_empty(ft260_t *ft260);

#endif //KEYLOGGER_FT260_H
