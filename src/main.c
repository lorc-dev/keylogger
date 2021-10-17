/* 
 * You can skip stdint.h and see how GCC defines things by running
 * arm-none-eabi-gcc -dM -E - < /dev/null
 */
//#include <stdint.h>
typedef __UINT32_TYPE__ uint32_t;

/* The gpio functions are described in the file:
 * src/rp2_common/hardware_gpio/gpio.c
 * Some are inlined in gpio.h (e.g. gpio_set_dir())
 */

#define REG(addr) *(volatile uint32_t*)(addr)


#define SIO_BASE 		0xd0000000 // see s2.3.1.7
#define SIO_GPIO_OUT		REG(SIO_BASE+0x010) // GPIO output value
#define SIO_GPIO_OUT_SET	REG(SIO_BASE+0x014) // GPIO output value set
#define SIO_GPIO_OUT_CLR	REG(SIO_BASE+0x018) // GPIO output value clear
#define SIO_GPIO_OE		REG(SIO_BASE+0x020) // GPIO output enable
#define SIO_GPIO_OE_SET		REG(SIO_BASE+0x024) // GPIO output enable set
#define SIO_GPIO_OE_CLR 	REG(SIO_BASE+0x028) // GPIO output enable clear

#define RESETS_BASE		0x4000c000 // see s2.14.3
#define RESETS_RESET		REG(RESETS_BASE+0x00)
#define RESETS_RESET_DONE	REG(RESETS_BASE+0x08)
#define RESETS_IO_BANK0		(1ul<<5)
#define RESETS_PAD_BANK0	(1ul<<8)

#define IO_BANK0_BASE 		0x40014000
#define IO_BANK0_GPIO25_CTRL 	REG(IO_BANK0_BASE+0x0cc)

#define PADS_BANK0_BASE 	0x4001c000 // see s2.19.6.3. Pad control register
#define PADS_BANK0_GPIO25	REG(PADS_BANK0_BASE+0x68)


#define GPIO_FUNC_SIO	5


#define LED 25

void delay(int n) // no particular timing
{
	for(int i =0 ; i< n; i++) {
		for(int j = 0; j< 1000; j++) {
			asm volatile ("nop");
		}
	}
}


// stub for exit
/*void exit(int status)
{
	while(1);
}
*/

int main()
{
	// inspired by Ada. Appears to be necessary, too.
	// Bring up the peripherals we require
	// See s2.14.1
	uint32_t reset_mask = RESETS_PAD_BANK0 | RESETS_IO_BANK0;
	RESETS_RESET &= ~reset_mask; // deassert the peripherals we want to use
	while((RESETS_RESET_DONE & reset_mask) != reset_mask); // wait until they are up again

	//PADS_BANK0_GPIO25 &= ~(1<<7); // clear output disable 
	//PADS_BANK0_GPIO25 &= ~(1<<6); // clear input enable

	IO_BANK0_GPIO25_CTRL = GPIO_FUNC_SIO; // init pin - select function SIO
	SIO_GPIO_OE_SET = 1ul << LED; // allow setting of output

	while(1) {
		SIO_GPIO_OUT_SET = 1ul << LED; 
		delay(1000);
		SIO_GPIO_OUT_CLR = 1ul << LED; // turn off the LED
		delay(1000);
	}

	return 0;
}
