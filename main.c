#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
//#include "hbi/hbi.h"
#include <stdio.h>

#define MASTER 1

int main(void) {
    stdio_init_all();

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);

	printf("started\n");

	spi_init(spi0, 1000000);

	if (!MASTER) {
		spi_set_slave(spi0, true);
	}

	gpio_set_function(2, GPIO_FUNC_SPI);
	gpio_set_function(3, GPIO_FUNC_SPI);
	gpio_set_function(4, GPIO_FUNC_SPI);
	gpio_set_function(5, GPIO_FUNC_SPI);

	if (MASTER) {
	uint8_t pingDat = 0;
	uint8_t returnDat;

	while (1) {
		uint32_t startTime = time_us_32();
		spi_write_blocking(spi0, &pingDat, 1);
		spi_read_blocking(spi0, 0, &returnDat, 1);
		if (returnDat != pingDat) {
			printf("Error: ping doesn't match");
		}

		uint32_t endTime = time_us_32() - startTime;
		printf("%i\n", endTime);
	}

	} else {


	}

    return 0;
}