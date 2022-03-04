//
// Created by Lorenz on 19/02/2022.
//

#ifndef KEYLOGGER_USB_H
#define KEYLOGGER_USB_H

#include <stdint.h>
#include <stdbool.h>
#include "addressmap.h"
#include "irq.h"

// Configuration options
#define USB_DEVICE_ADDRESS          0x1     // Only one device, so we can hard code the address
#define USB_RESET_DELAY             100     // 50 ms according to the USB spec, some devices take longer
#define USB_CONFIGURATION_NUMBER    0x1     // Assume there is only one configuration, but if not, the number can be chosen here


/////////////////////////
/// USB controller hw ///
/////////////////////////

// USB registers (See Table 407: List of USB registers)
typedef struct {
    uint32_t volatile  dev_addr_ep_ctrl;        // Device address and endpoint control
    uint32_t volatile  host_int_ep_addr_ctrl[15];   // Interrupt endpoint 1-15. Only for HOST mode
    uint32_t volatile  main_ctrl;               // Main control register
    uint32_t volatile  sof_rw;                  // Set the SOF (Start of Frame) frame number in the host controller.
    uint32_t volatile  sof_rd;                  // Read the last SOF (Start of Frame) frame number seen.
    uint32_t volatile  sie_ctrl;                // SIE control register
    uint32_t volatile  sie_status;              // SIE status register
    uint32_t volatile  int_ep_ctrl;             // Interrupt endpoint control register
    uint32_t volatile  buff_status;             // Buffer status register
    uint32_t volatile  buff_cpu_should_handle;  // Which of the double buffers should be handled.
    uint32_t volatile  ep_abort;                // (Device only)
    uint32_t volatile  ep_abort_done;           // (Device only)
    uint32_t volatile  ep_stall_arm;            // (Device only)
    uint32_t volatile  nak_poll;                // Sets wait time  before trying again if the device replies with a NAK.
    uint32_t volatile  ep_status_stall_nak;     // (Device only)
    uint32_t volatile  usb_muxing;              // Where to connect the USB controller. Should be to_phy by default.
    uint32_t volatile  usb_pwr;                 // Overrides for the power signals when VBUS signals aren't hooked up to GPIO
    uint32_t volatile  usbphy_direct;           // Direct control of the USB phy.
    uint32_t volatile  usbphy_direct_override;  // Override enable for each control in usbphy_direct
    uint32_t volatile  usbphy_trim;             // Used to adjust trim values of USB phy pull down resistors.
    uint32_t volatile  linestate_tuning;        // Not datasheet, stolen from sdk
    uint32_t volatile  intr;                    // Raw interrupts
    uint32_t volatile  inte;                    // Interrupt enable
    uint32_t volatile  intf;                    // Interrupt Force
    uint32_t volatile  ints;                    // Interrupt status
} usb_hw_t;

// DPSRAM 4kB Host function (See Table 404: DPSRAM layout)
typedef struct {
    volatile uint8_t setup_packet[8];

    // Interrupt endpoint control 1-15
    struct int_ep_ctrl {
        uint32_t volatile ctrl;
        uint32_t volatile spare;
    } int_ep_ctrl[15];

    uint32_t volatile epx_buff_ctrl;
    uint32_t volatile _spare0;

    // Interrupt endpoint buffer control 1-15
    struct int_ep_buff_ctrl {
        uint32_t volatile ctrl;
        uint32_t volatile spare;
    } int_ep_buff_ctrl[15];

    uint32_t volatile epx_ctrl;

    uint8_t _spare1[124];

    // 0x180-0xfff (0xE80 bytes)
    uint8_t data_buffers[0x1000 - 0x180];
} usb_host_dpsram_t;


#define usb_hw ((usb_hw_t *)USBCTRL_REGS_BASE)
#define usb_host_dpsram ((usb_host_dpsram_t *)USBCTRL_DPRAM_BASE)

// Device address and endpoint control register (See Table 408: ADDR_ENDP Register)
#define USB_ADDR_ENDP_ENDPOINT_BITS     (0xFul << 16)
#define USB_ADDR_ENDP_ADDRESS_BITS      (0x3F)
#define USB_ADDR_ENDP_ENDPOINT_LSB      16
#define USB_ADDR_ENDP_ADDRESS_LSB       0

// Interrupt endpoint N register (See Table 409: ADDR_ENDP)
#define USB_ADDR_ENDPN_INTEP_PREAMBLE_BIT   (1ul << 26)
#define USB_ADDR_ENDPN_INTEP_DIR_BIT        (1ul << 25)
#define USB_ADDR_ENDPN_ENDPOINT_BITS        (0xFul << 16)
#define USB_ADDR_ENDPN_ENDPOINT_LSB         16


// Endpoint control register (See Table 405: Endpoint control register layout)
#define USB_EP_CTRL_ENABLE_BIT              (1ul << 31)
#define USB_EP_CTRL_DOUBLE_BUFFERED_BIT     (1ul << 30)
#define USB_EP_CTRL_INT_BUFFER_BIT          (1ul << 29)
#define USB_EP_CTRL_INT_2_BUFFER_BIT        (1ul << 28)
#define USB_EP_CTRL_TYPE_BITS               (3ul << 26)
#define USB_EP_CTRL_HOST_INT_INTERVAL_BITS  (0x3FFul << 16)
#define USB_EP_CTRL_ADDR_BASE_OFFSET_BITS   (0x3FFul << 6)
#define USB_EP_CTRL_TYPE_LSB                26
#define USB_EP_CTRL_HOST_INT_INTERVAL_LSB   16
#define USB_EP_CTRL_ADDR_BASE_OFFSET_LSB    6

// Buffer control register (See Table 406: Buffer control register layout)
#define USB_BUFF_CTRL_BUFF1_FULL_BIT                (1ul << 31)
#define USB_BUFF_CTRL_BUFF1_LAST_BIT                (1ul << 30)
#define USB_BUFF_CTRL_BUFF1_DATA_PID_BIT            (1ul << 29)
#define USB_BUFF_CTRL_DOUBLE_BUFF_OFFSET_BITS       (3ul << 27)
#define USB_BUFF_CTRL_AVAILABLE_1_BIT               (1ul << 26)
#define USB_BUFF_CTRL_BUFF1_TRANSFER_LENGTH_BITS    (0x3FFul << 16)
#define USB_BUFF_CTRL_BUFF0_FULL_BIT                (1ul << 15)
#define USB_BUFF_CTRL_BUFF0_LAST_BIT                (1ul << 14)
#define USB_BUFF_CTRL_BUFF0_DATA_PID_BIT            (1ul << 13)
#define USB_BUFF_CTRL_STALL_RECEIVED_BIT            (1ul << 11)
#define USB_BUFF_CTRL_AVAILABLE_0_BIT               (1ul << 10)
#define USB_BUFF_CTRL_BUFF0_TRANSFER_LENGTH_BITS    0x3FF
#define USB_BUFF_CTRL_BUFF1_TRANSFER_LENGTH_LSB     16
#define USB_BUFF_CTRL_BUFF0_TRANSFER_LENGTH_LSB     0

// USB muxing register (See Table 423: USB_MUXING Register)
#define USB_MUXING_SOFTCON_BIT          (1ul << 3)
#define USB_MUXING_TO_DIGITAL_PAD_BIT   (1ul << 2)
#define USB_MUXING_TO_EXTPHY_BIT        (1ul << 1)
#define USB_MUXING_TO_PHY_BIT           1ul

// USB pwr register (See Table 424: USB_PWR Register)
#define USB_PWR_OVERCURR_DETECT_EN_BIT          (1ul << 5)
#define USB_PWR_OVERCURR_DETECT_BIT             (1ul << 4)
#define USB_PWR_VBUS_DETECT_OVERRIDE_EN_BIT     (1ul << 3)
#define USB_PWR_VBUS_DETECT_BIT                 (1ul << 2)
#define USB_PWR_VBUS_EN_OVERRIDE_EN_BIT         (1ul << 1)
#define USB_PWR_VBUS_EN_BIT                     1ul

// Main control register (See Table 410: MAIN_CTRL Register)
#define USB_MAIN_CTRL_HOST_NDEVICE_BIT      (1ul << 1)  // Device mode = 0, Host mode = 1
#define USB_MAIN_CTRL_CONTROLLER_EN_BIT     1ul

// SIE control register (See Table 413: SIE_CTRL Register)
#define USB_SIE_CTRL_EP0_INT_1BUF_BIT   (1ul << 29)
#define USB_SIE_CTRL_DIRECT_EN_BIT      (1ul << 26)
#define USB_SIE_CTRL_DIRECT_DP_BIT      (1ul << 25)
#define USB_SIE_CTRL_DIRECT_DM_BIT      (1ul << 24)
#define USB_SIE_CTRL_TRANSCEIVER_PD_BIT (1ul << 18)
#define USB_SIE_CTRL_PULLDOWN_EN_BIT    (1ul << 15)
#define USB_SIE_CTRL_RESET_BUS_BIT      (1ul << 13)
#define USB_SIE_CTRL_VBUS_EN_BIT        (1ul << 11)
#define USB_SIE_CTRL_KEEP_ALIVE_EN_BIT  (1ul << 10)
#define USB_SIE_CTRL_SOF_EN_BIT         (1ul << 9)
#define USB_SIE_CTRL_SOF_SYNC_BIT       (1ul << 8)
#define USB_SIE_CTRL_PREAMBLE_EN_BIT    (1ul << 6)
#define USB_SIE_CTRL_STOP_TRANS_BIT     (1ul << 4)
#define USB_SIE_CTRL_RECEIVE_DATA_BIT   (1ul << 3)
#define USB_SIE_CTRL_SEND_DATA_BIT      (1ul << 2)
#define USB_SIE_CTRL_SEND_SETUP_BIT     (1ul << 1)
#define USB_SIE_CTRL_START_TRANS_BIT    (1ul)

// Interrupt enable register (See Table 429: INTE Register)
#define USB_INTE_EP_STACK_NAK_BIT       (1ul << 19)
#define USB_INTE_ABORT_DONE_BIT         (1ul << 18)
#define USB_INTE_BUS_RESET_BIT          (1ul << 12)
#define USB_INTE_VBUS_DETECT_BIT        (1ul << 11)
#define USB_INTE_STALL_BIT              (1ul << 10)
#define USB_INTE_ERROR_CRC_BIT          (1ul << 9)
#define USB_INTE_ERROR_BIT_STUFF_BIT    (1ul << 8)
#define USB_INTE_ERROR_RX_OVERFLOW_BIT  (1ul << 7)
#define USB_INTE_ERROR_RX_TIMEOUT_BIT   (1ul << 6)
#define USB_INTE_ERROR_DATA_SEQ_BIT     (1ul << 5)
#define USB_INTE_BUFF_STATUS_BIT        (1ul << 4)
#define USB_INTE_TRANS_COMPLETE_BIT     (1ul << 3)
#define USB_INTE_HOST_SOF_BIT           (1ul << 2)
#define USB_INTE_HOST_RESUME_BIT        (1ul << 1)
#define USB_INTE_HOST_CONN_DIS_BIT      1ul

// Interrupt status after masking & forcing register (See Table 431: INTS Register)
#define USB_INTS_EP_STACK_NAK_BIT       (1ul << 19)
#define USB_INTS_ABORT_DONE_BIT         (1ul << 18)
#define USB_INTS_BUS_RESET_BIT          (1ul << 12)
#define USB_INTS_VBUS_DETECT_BIT        (1ul << 11)
#define USB_INTS_STALL_BIT              (1ul << 10)
#define USB_INTS_ERROR_CRC_BIT          (1ul << 9)
#define USB_INTS_ERROR_BIT_STUFF_BIT    (1ul << 8)
#define USB_INTS_ERROR_RX_OVERFLOW_BIT  (1ul << 7)
#define USB_INTS_ERROR_RX_TIMEOUT_BIT   (1ul << 6)
#define USB_INTS_ERROR_DATA_SEQ_BIT     (1ul << 5)
#define USB_INTS_BUFF_STATUS_BIT        (1ul << 4)
#define USB_INTS_TRANS_COMPLETE_BIT     (1ul << 3)
#define USB_INTS_HOST_SOF_BIT           (1ul << 2)
#define USB_INTS_HOST_RESUME_BIT        (1ul << 1)
#define USB_INTS_HOST_CONN_DIS_BIT      1ul

// SIE status register (See Table 414: SIE_STATUS Register)
#define USB_SIE_STATUS_DATA_SEQ_ERROR_BIT   (1ul << 31)
#define USB_SIE_STATUS_ACK_REC_BIT          (1ul << 30)
#define USB_SIE_STATUS_STALL_REC_BIT        (1ul << 29)
#define USB_SIE_STATUS_NAK_REC_BIT          (1ul << 28)
#define USB_SIE_STATUS_RX_TIMEOUT_BIT       (1ul << 27)
#define USB_SIE_STATUS_RX_OVERFLOW_BIT      (1ul << 26)
#define USB_SIE_STATUS_BIT_STUFF_ERROR_BIT  (1ul << 25)
#define USB_SIE_STATUS_CRC_ERROR_BIT        (1ul << 24)
#define USB_SIE_STATUS_TRANS_COMPLETE_BIT   (1ul << 18)
#define USB_SIE_STATUS_RESUME_BIT           (1ul << 11)
#define USB_SIE_STATUS_VBUS_OVER_CURR_BIT   (1ul << 10)
#define USB_SIE_STATUS_SPEED_BITS           (3ul << 8)
#define USB_SIE_STATUS_SUSPENDED_BIT        (1ul << 4)
#define USB_SIE_STATUS_LINE_STATE_BITS      (3ul << 2)
#define USB_SIE_STATUS_SPEED_LSB            8
#define USB_SIE_STATUS_LINE_STATE_LSB       2


////////////////////
/// USB protocol ///
////////////////////

typedef enum {
    usb_disconnected,
    usb_low_speed,
    usb_full_speed
} dev_speed_t;

typedef enum {
    usb_bm_request_type_recipient_device = 0,
    usb_bm_request_type_recipient_interface,
    usb_bm_request_type_recipient_endpoint,
    usb_bm_request_type_recipient_other
} usb_bm_request_type_recipient_t;

typedef enum {
    usb_bm_request_type_type_standard = 0,
    usb_bm_request_type_type_class,
    usb_bm_request_type_type_vendor
} usb_bm_request_type_type_t;

typedef enum {
    usb_bm_request_type_direction_host_to_dev = 0,
    usb_bm_request_type_direction_dev_to_host
} usb_bm_request_type_direction_t;

typedef enum {
    usb_setup_req_b_req_type_get_status = 0,
    usb_setup_req_b_req_type_clear_feature,
    usb_setup_req_b_req_type_reserved,
    usb_setup_req_b_req_type_set_feature,
    usb_setup_req_b_req_type_reserved_2,
    usb_setup_req_b_req_type_set_address,
    usb_setup_req_b_req_type_get_descriptor,
    usb_setup_req_b_req_type_set_descriptor,
    usb_setup_req_b_req_type_get_configuration,
    usb_setup_req_b_req_type_set_configuration,
    usb_setup_req_b_req_type_get_interface,
    usb_setup_req_b_req_type_set_interface,
    usb_setup_req_b_req_type_synch_frame
} usb_b_request_t;

typedef enum {
    usb_descriptor_types_device = 1,
    usb_descriptor_types_configuration,
    usb_descriptor_types_string,
    usb_descriptor_types_interface,
    usb_descriptor_types_endpoint,
    usb_descriptor_types_device_qualifier,
    usb_descriptor_types_other_speed_configuration,
    usb_descriptor_types_interface_power,
    usb_descriptor_types_on_the_go
} usb_descriptor_types_t;

typedef enum {
    usb_data_flow_types_control_transfer = 0,
    usb_data_flow_types_isochronous_transfer,
    usb_data_flow_types_bulk_transfer,
    usb_data_flow_types_interrupt_transfer
} usb_data_flow_types_t;

typedef struct {
    bool connected;
    bool enumerated;
    bool hid_driver_loaded;

    dev_speed_t speed;
    uint8_t address;

    uint8_t local_interrupt_endpoint_number;

    // Device descriptor
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t max_packet_size_ep_0;
    uint8_t configuration_count;

    // Configuration descriptor
    uint8_t interface_count;    // Assume only 1 configuration

    // Interface descriptor
    uint8_t interface_number_hid;   // Only support the HID class

    // Endpoint descriptor
    uint8_t interrupt_out_endpoint_number;
    uint16_t interrupt_out_endpoint_max_packet_size;
    uint8_t interrupt_out_endpoint_polling_interval;

} usb_device_t;

// Setup data format (See Table 9-2 Format of Setup Data (usb20 spec))
typedef struct __attribute__ ((packed)) {
    union {
        struct {
            uint8_t recipient : 5;  // Use: usb_bm_request_type_recipient_t
            uint8_t type : 2;       // use: usb_bm_request_type_type_t
            uint8_t direction : 1;  // Use: usb_bm_request_type_direction_t
        } bm_request_type_bits;

        uint8_t bm_request_type;
    };

    uint8_t  b_request;  // Specific request (refer to Table 9-3)
    uint16_t w_value;    // Word-sized field that varies according to request
    uint16_t w_index;    // Word-sized field that varies according to request; typically used to pass an index or offset
    uint16_t w_length;   // Number of bytes to transfer if there is a data stage
} usb_setup_data_t;

// Standard Device Descriptor (See Table 9-8 (usb20 spec))
typedef struct __attribute__ ((packed)) {
    uint8_t b_length;           // Size of this descriptor in bytes
    uint8_t b_descriptor_type;  // Constant 1
    uint16_t bcd_usb;           // Usb Spec release number, BCD encoded
    uint8_t b_device_class;     // Class code
    uint8_t b_device_sub_class; // Subclass code
    uint8_t b_device_protocol;  // Protocol code
    uint8_t b_max_packet_size_0;// Maximum packet for endpoint 0 (8,16,32 or 64)
    uint16_t id_vendor;         // Vendor ID
    uint16_t id_product;        // Product ID
    uint16_t bcd_device;        // Device release number, BCD encoded
    uint8_t i_manufacturer;     // Index of string descriptor describing manufacturer
    uint8_t i_product;          // Index of string descriptor describing device serial number
    uint8_t i_serial_number;    // Index of string descriptor describing the device's serial number
    uint8_t b_num_configurations;   // Number of possible configurations
} usb_device_descriptor_t;

// Standard Configuration Descriptor (See Table 9-10 (usb20 spec))
typedef struct __attribute__ ((packed)) {
    uint8_t b_length;           // Size of this descriptor in bytes
    uint8_t b_descriptor_type;  // Constant 2
    uint16_t w_total_length;    // Total length of data returned for this configuration
    uint8_t b_num_interfaces;   // Number of interfaces supported by this configuration
    uint8_t b_configuration_value;  // Value to use as an argument to the SetConfiguration() request to select this configuration
    uint8_t i_configuration;    // Index of string descriptor describing this configuration
    uint8_t bm_attributes;      // Configuration characteristics
    uint8_t b_max_power;        // Maximum power consumption, expressed in 2mA units

} usb_configuration_descriptor_t;

// Standard Interface Descriptor (See Table 9-12 (usb20 spec))
typedef struct __attribute__ ((packed)) {
    uint8_t b_length;               // Size of this descriptor in bytes
    uint8_t b_descriptor_type;      // Constant 4
    uint8_t b_interface_number;     // Number of this interface
    uint8_t b_alternate_setting;    // Value used to select this alternate setting for the interface identified in the prior field
    uint8_t b_num_endpoints;        // Number of endpoints used by this interface
    uint8_t b_interface_class;      // Class code
    uint8_t b_interface_sub_class;  // Subclass code
    uint8_t b_interface_protocol;   // Protocol code
    uint8_t i_interface;            // Index of string descriptor describing this interface
} usb_interface_descriptor_t;

// Standard Endpoint Descriptor (See Table 9-13 (usb20 spec))
typedef struct __attribute__ ((packed)) {
    uint8_t b_length;               // Size of this descriptor in bytes
    uint8_t b_descriptor_type;      // Constant 5
    union {
        struct {
            uint8_t endpoint_number : 4;
            uint8_t reserved : 3;
            uint8_t direction : 1;  // 1 = IN, 0 = OUT
        } b_endpoint_address_bits;
        uint8_t b_endpoint_address; // The address of the endpoint on the USB device
    };
    union {
        struct {
            uint8_t transfer_type : 2;
            uint8_t synchronization_type : 2;
            uint8_t usage_type : 2;
            uint8_t reserved : 2;
        } bm_attributes_bits;
        uint8_t bm_attributes;      // This field describes the endpoint’s attributes when it is configured using the bConfigurationValue
    };
    uint16_t w_max_packet_size;     // Maximum packet size this endpoint is capable of sending or receiving when this configuration is selected.
    uint8_t b_interval;             // Interval for polling endpoint for data transfers. Expressed in frames or microframes depending on the device operating speed
} usb_endpoint_descriptor_t;

struct endpoint_struct {
    bool receive;   // 1 = receive, 0 = transmit
    bool last_buffer;
    bool active;
    bool setup;

    uint8_t buffer_selector;    // See RP2040-E4 bug

    uint8_t device_address;
    uint8_t endpoint_number;    // Endpoint on device
    uint8_t pid;

    uint8_t interrupt_number;   // Internal endpoint number (0-15) 0 = epx

    uint32_t volatile * endpoint_control;   // Endpoint control register
    uint32_t volatile * buffer_control;     // Buffer control register
    uint8_t * dps_data_buffer;              // Buffer pointer in dpsram
    uint8_t * mem_data_buffer;

    usb_data_flow_types_t transfer_type;
    uint16_t w_max_packet_size;
    uint16_t transferred_bytes;
    //uint16_t remaining_bytes;
    uint16_t total_len;
    uint16_t transfer_size;
    uint8_t interrupt_interval;
};

// USB descriptors
#define USB_DESCRIPTOR_B_LENGTH_OFFSET                  0ul
#define USB_DESCRIPTOR_B_DESCRIPTOR_TYPE_OFFSET         1ul
#define USB_ENDPOINT_DESCRIPTOR_MAX_PACKET_SIZE_BITS    0x7FFul

// USB class codes
#define USB_CLASS_CODE_HID  0x03

///////////////////////////
/// Function prototypes ///
///////////////////////////
void usb_init(void);
static void usb_init_endpoints(void);
static void usb_irq(void);
static inline dev_speed_t device_speed(void);
void usb_device_attach_handler(void);
void usb_device_detach_handler(void);
void usb_enum_device(usb_device_t * device);
static void usb_reset_bus(void);
static void usb_setup_send(uint8_t device_address, usb_setup_data_t *setup_packet);
void usb_endpoint_init(struct endpoint_struct *endpoint, uint8_t device_address, uint8_t endpoint_number, uint8_t direction, uint16_t w_max_packet_size, usb_data_flow_types_t transfer_type, uint8_t b_interval);
void usb_endpoint_transfer(uint8_t device_address, struct endpoint_struct *endpoint, uint16_t endpoint_number,uint8_t * buffer, uint16_t buffer_len, uint8_t direction);
void usb_send_control_transfer(uint8_t device_address, usb_setup_data_t *setup_packet, uint8_t * data);
static bool usb_endpoint_transfer_continue(struct endpoint_struct *endpoint);
static void usb_endpoint_reset(struct endpoint_struct *endpoint);
void usb_endpoint_transfer_buffer(struct endpoint_struct * endpoint);
static void usb_handle_buff_status(void);
static void handle_transfer_complete(void);
struct endpoint_struct * usb_get_endpoint(uint8_t endpoint_number);

#endif //KEYLOGGER_USB_H
