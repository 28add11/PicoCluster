#include "pico/stdlib.h"

void main(void){

	//__breakpoint();

	//asm("bkpt #70");

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	while (1) {
		gpio_put(PICO_DEFAULT_LED_PIN, 1);
    	sleep_ms(250);
    	gpio_put(PICO_DEFAULT_LED_PIN, 0);
		sleep_ms(250);
	}

	return;
}
