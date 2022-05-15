#include "include/hardware/gpio.h"
#include "include/hardware/sio.h"
#include "include/runtime/runtime.h"
#include "include/hardware/uart.h"
#include "include/hardware/spi.h"
#include "include/hardware/i2c.h"
#include "include/drivers/ssd1306.h"
#include "include/hardware/timer.h"
#include "include/hardware/usb.h"
#include "include/drivers/usb_host_hid.h"
#include "include/drivers/usb_hid.h"
#include "include/events/events.h"
#include "include/drivers/sd_spi.h"
#include "include/storage/storage.h"
#include "include/lib/graphics/graphics.h"
#include "include/drivers/ft260.h"
#include "include/ui/ui.h"


int main(void) {
    runtime_init(); // TODO: Call runtime_init() from crt0.s before main ?

	gpio_enable();

    event_init_queue();

    // UART
    uart_init(uart0_hw,9600);
    gpio_set_function(0,GPIO_FUNC_UART);

    // FT260
    ft260_t ft260;
    ft260_init(&ft260, i2c0_hw, 8, 9, 15);

    // I2C for the SSD1306 display
    i2c_init(i2c1_hw, 100000);
    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);

    // SSD1306 display
    ssd1306_t ssd1306;
    ssd1306_init(&ssd1306, i2c1_hw, 0x3C, 128,32);

    // Graphics
    graphics_display_t display = graphics_init(&ssd1306);

    // USB host controller
    usb_device_t usb_device;
    usb_init(&usb_device);

    // Sd card
    sd_spi_t sd;
    sd_spi_init(&sd,spi0_hw, 17, 19, 20, 18, 1);

    // Storage
    storage_t storage;
    storage_init(&storage, &sd);

    // HID report parser
    usb_hid_keyboard_report_parser_t hid_parser;
    hid_report_parser_init(&hid_parser, hid_keymap_qwerty);
    uint8_t pressed_keys[6];
    usb_hid_boot_keyboard_input_report_t hid_report;
    usb_hid_boot_keyboard_output_report_t output_report;

    // UI
    ui_t ui;
    ui_init(&ui, &display, 14, 2, &storage, &usb_device, &hid_parser);

    // Loading screen
    ui_menu_loading_screen(&ui);
    wait_ms(500);

    // Words per minute
    uint8_t wpm = 0;
    uint8_t char_count = 0;
    uint64_t time_interval_start = get_time();
    uint64_t time_interval_now = time_interval_start;

    // Loop forever
	while (1) {
        event_task();
        graphics_task(&display);
        sd_spi_task(&sd);
        storage_task(&storage);
        ui_task(&ui);

        // Read all HID input reports from the usb host report queue
        while (!usb_host_hid_report_queue_is_empty()) {
            // Get the report from the queue
            hid_report = usb_host_hid_report_dequeue();

            // Send the report to the host (via ft260 usb <-> hid bridge)
            ft260_send_input_report(&ft260, &hid_report);

            // Parse the report
            int chars = hid_report_parse(&hid_parser, &hid_report, pressed_keys);
            // Loop through all the pressed keys
            for (int i = 0; i < chars; i++) {
                // Store the character
                storage_store_byte(&storage, pressed_keys[i]);
                // Update the UI with the new data
                ui_menu_live_output_set_data(&ui, pressed_keys[i]);
            }

            // Update the char count var to use in the WPM calculation
            char_count += chars;
        }

        // Send an output report to the keyboard if a new one is available
        if (ft260_get_output_report(&ft260, &output_report)) {
           usb_host_hid_send_output_report(&output_report);
        }

        // Words per minute
        time_interval_now = get_time();
        if (time_interval_now - time_interval_start >= 5000000) {  // Check if 15 seconds have passed
            // Calculate the new WPM value
            wpm = (uint8_t)(char_count * 12 / 5);        // 5 chars/word and 4 * 15 s / 60 s (min)

            // Update the UI with the new data
            ui_menu_user_stats_set_data(&ui, wpm);

            // Reset
            time_interval_start = time_interval_now;
            char_count = 0;
        }
	}

	return 0;
}
