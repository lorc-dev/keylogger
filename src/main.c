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
#include "include/events/events.h"
#include "include/drivers/sd_spi.h"

#define LED 25

void delay(int n) // no particular timing
{
	for(int i =0 ; i< n; i++) {
		for(int j = 0; j< 1000; j++) {
			asm volatile ("nop");
		}
	}
}

void toggle(void) {
    sio_toggle(LED);
}

int main()
{
    runtime_init(); // TODO: Call runtime_init() from crt0.s before main ?

	gpio_enable();

    event_init_queue();

	// SIO
    gpio_set_function(25, GPIO_FUNC_SIO);
    sio_init(LED);
    sio_set_dir(LED,OUTPUT);

    // UART
    //uart_init(uart0_hw,9600);
   // gpio_set_function(1,GPIO_FUNC_UART);
    //gpio_set_function(0,GPIO_FUNC_UART);
    uint8_t test[5] = {0,1,2,3,4};

    // SPI
//    spi_init(spi0_hw,50000);
//    gpio_set_function(2,GPIO_FUNC_SPI);
//    gpio_set_function(3,GPIO_FUNC_SPI);
//    gpio_set_function(4,GPIO_FUNC_SPI);

    // I2C
  /*  i2c_init(i2c1_hw, 100000);
    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);
    uint8_t buffer[1024];
    ssd1306_t ssd1306 = ssd1306_init(i2c1_hw, 0x3C, 128,32, buffer);

    for(int i = 0; i < 1024; i++){
        buffer[i] = 0;
    }
    buffer[0] = 8;
    buffer[1] = 8;
    buffer[2] = 8;
    buffer[3] = 8;

    buffer[100] = 8;
    buffer[101] = 16;
    buffer[102] = 32;
    buffer[103] = 64;

    buffer[129] = 255;
    buffer[130] = 127;
    buffer[131] = 63;
    buffer[132] = 31;

    for(int y = 0; y < 32; y++)
        for(int x = 0; x < 128; x++)
            if (x == y)
                ssd1306_set_pixel(&ssd1306, x+y/3, y, true);

    wait_ms(100);
    ssd1306_display(&ssd1306);*/

    //usb_init();

   gpio_set_function(12, GPIO_FUNC_SIO);
    gpio_set_pulldown(12, false);
    gpio_set_pullup(12,true);
   sio_init(12);
   sio_set_dir(12,INPUT);

//    gpio_set_irq_enabled(12,gpio_irq_event_edge_high, toggle);
//    gpio_set_irq_enabled(12,gpio_irq_event_edge_low, toggle);

    bool sd_init = false;
    uint8_t sd_buff[512];

    for(int i = 0; i < 512; i++) {
        sd_buff[i] = (uint8_t)i;
    }

	while(1) {
//	    spi_write(spi0_hw, &test,5);
        //uart_puts(uart0_hw, "Dit is een test");
//	    uart_write(uart0_hw,&test,5);
	    //i2c_write(i2c0_hw, 0x3C, &test, 5, true, true);
        event_task();
        if (!sd_init && sio_get(12)) {
            sd_spi_t sd = sd_spi_init(spi0_hw, 5,3,4,2);
            wait_ms(50);
            sd_spi_write_block(&sd, sd_buff, 1);
            wait_ms(500);
            sd_spi_read_block(&sd, sd_buff,1);
            sd_init = true;
        }

		sio_put(LED,1);
        wait_ms(100);   //delay(150);
        sio_put(LED,0);
        wait_ms(100);   //delay(150);
	}

	return 0;
}
