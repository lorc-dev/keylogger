//
// Created by Lorenz on 4/03/2022.
//

#ifndef KEYLOGGER_EVENTS_H
#define KEYLOGGER_EVENTS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define EVENT_QUEUE_SIZE    32  // Max items in the event queue, increase if necessary

typedef enum {
    event_usb_device_attached,
    event_usb_device_detached,
    event_usb_host_hid_load_driver,
    event_usb_host_hid_report_available,
    event_storage_initialized,
    event_storage_block_written,
    event_sd_card_disconnected,
} event_type_t;

typedef struct {
    event_type_t event_type;
    void (* callback) (void);
} event_t;


// Function prototypes
void event_init_queue(void);
void event_add(event_type_t event_type, void (* callback) (void));
void event_task(void);
static void event_enqueue(event_t event);
static event_t event_dequeue(void);
static inline bool event_queue_is_full(void);
static inline bool event_queue_is_empty(void);

#endif //KEYLOGGER_EVENTS_H
