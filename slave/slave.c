/*
//	Code for "Slave" pico in cluster
//
//	Written by Nicholas West, 2024
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"
//#include "hbi/hbi.h"
#include <stdio.h>
#include <stdlib.h>

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

	spi_set_slave(spi0, true);
	
	gpio_set_function(2, GPIO_FUNC_SPI);
	gpio_set_function(3, GPIO_FUNC_SPI);
	gpio_set_function(4, GPIO_FUNC_SPI);
	gpio_set_function(5, GPIO_FUNC_SPI);

	uint8_t instruction;
	uint8_t data;

	while (1) {
		spi_read_blocking(spi0, 0, &instruction, 1);

		switch (instruction)
		{

		case 0: // Alignment reset
			spi_deinit(spi0);
			gpio_put(LED_PIN, 1);
			sleep_ms(10); // Give time for any in-progress transfers to stop
			gpio_put(LED_PIN, 0);
			spi_init(spi0, 1000000);
			spi_set_slave(spi0, true);
			break;
			
		case 1: // Ping
			spi_read_blocking(spi0, 0, &data, 1);
			spi_write_blocking(spi0, &data, 1);
			break;

		case 2: // Malloc
			// Get the size of the buffer to allocate
			; // Stupid noop to avoid errors
			size_t size;
			spi_read_blocking(spi0, 0, &size, sizeof(size_t));

			uint8_t *adress = malloc(size);
			spi_write_blocking(spi0, &adress, sizeof(uint8_t *));
		
		default:
			break;
		}
	}

    return 0;
}