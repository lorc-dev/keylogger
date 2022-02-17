#include "include/hardware/gpio.h"
#include "include/hardware/sio.h"
#include "include/runtime/runtime.h"
#include "include/hardware/uart.h"
#include "include/hardware/spi.h"
#include "include/hardware/i2c.h"
#include "include/drivers/ssd1306.h"

#define LED 25

void delay(int n) // no particular timing
{
	for(int i =0 ; i< n; i++) {
		for(int j = 0; j< 1000; j++) {
			asm volatile ("nop");
		}
	}
}


int main()
{
    runtime_init(); // TODO: Call runtime_init() from crt0.s before main ?

	gpio_enable();

	// SIO
    gpio_set_function(25, GPIO_FUNC_SIO);
    sio_init(LED);
    sio_set_dir(LED,OUTPUT);

    // UART
    uart_init(uart0_hw,9600);
    gpio_set_function(1,GPIO_FUNC_UART);
    gpio_set_function(0,GPIO_FUNC_UART);
    uint8_t test[5] = {0,1,2,3,4};

    // SPI
    spi_init(spi0_hw,10000000);
    gpio_set_function(2,GPIO_FUNC_SPI);
    gpio_set_function(3,GPIO_FUNC_SPI);
    gpio_set_function(4,GPIO_FUNC_SPI);

    // I2C
    i2c_init(i2c0_hw, 100000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    uint8_t buffer[1024];
    ssd1306_t ssd1306 = ssd1306_init(i2c0_hw, 0x3C, 128,32, buffer);

    for(int i = 0; i < 1024; i++){
        buffer[i] = 0;
    }
    buffer[5] = 255;

	while(1) {
	    spi_write(spi0_hw, &test,5);
        uart_puts(uart0_hw, "Dit is een test");
	    uart_write(uart0_hw,&test,5);
	    i2c_write(i2c0_hw, 0x3C, &test, 5, true, true);
	    ssd1306_display(&ssd1306);
		sio_put(LED,1);
		delay(500);
        sio_put(LED,0);
		delay(500);
	}

	return 0;
}
