#include "include/hardware/gpio.h"
#include "include/hardware/sio.h"
#include "include/runtime/runtime.h"


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
    gpio_set_function(25, GPIO_FUNC_SIO);

    sio_init(LED);
    sio_set_dir(LED,OUTPUT);


	while(1) {
		sio_put(LED,1);
		delay(100);
        sio_put(LED,0);
		delay(100);
	}

	return 0;
}
