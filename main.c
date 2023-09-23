#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hbi/hbi.h"
#include <stdio.h>

struct hbi mainLink;

int main(void) {
    stdio_init_all();

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);

    start_hbi(pio0, 2, &mainLink); //start at pin 2 in case we want to debug with UART

	printf("started\n");

	while (1) {
		//uint32_t pingData = 1;
		//gpio_put(LED_PIN, 1);
		//printf("pingTime: %i\n", *(int *)sendReq(mainLink, 1, &pingData));
		//
		//sleep_ms(500);
		//gpio_put(LED_PIN, 0);
		tight_loop_contents();
	}
    return 0;
}