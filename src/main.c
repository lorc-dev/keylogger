#include "include/hardware/gpio.h"
#include "include/hardware/sio.h"


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
	gpio_enable();
    gpio_set_function(25, GPIO_FUNC_SIO);

    sio_init(LED);
    sio_set_dir(LED,OUTPUT);


	while(1) {
		sio_put(LED,1);
		delay(500);
        sio_put(LED,0);
		delay(500);
	}

	return 0;
}
