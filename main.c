#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
//#include "hbi/hbi.h"
#include <stdio.h>
#include <stdlib.h>
// Note: defined in CMAKE for build sake
//#define MASTER 1

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

	if (!MASTER) { // Slave mode
		spi_set_slave(spi0, true);
	}

	gpio_set_function(2, GPIO_FUNC_SPI);
	gpio_set_function(3, GPIO_FUNC_SPI);
	gpio_set_function(4, GPIO_FUNC_SPI);
	gpio_set_function(5, GPIO_FUNC_SPI);

	if (MASTER) { // Master mode

	while(1) {
		uint8_t returnDat; // Send a test ping
		uint8_t pingDat = 0;
		uint32_t startTime = time_us_32();

		spi_write_blocking(spi0, (uint8_t *)(0), 1); // instruction first
		spi_write_blocking(spi0, &pingDat, 1);
		spi_read_blocking(spi0, 0, &returnDat, 1);
		if (returnDat != 255) {
			printf("Error: ping doesn't match\nGot back %i instead\n", returnDat);
		}

		uint32_t endTime = time_us_32() - startTime;
		printf("Ping time: \t%i\n\n", endTime);
		pingDat++;

		/*
		// Now we test the malloc
		spi_write_blocking(spi0, (uint8_t *)(1), 1); // Instruction
		spi_write_blocking(spi0, (uint8_t *)(255), sizeof(size_t)); // Size to allocate

		uint8_t *returnedPoint;
		spi_read_blocking(spi0, 0, &returnedPoint, sizeof(uint8_t *));
		printf("Pointer result: \t%p\n", returnedPoint);
		*/

		sleep_ms(100);
	}

	} else {
	uint8_t instruction;
	uint8_t data;

	while (1) {
		spi_read_blocking(spi0, 0, &instruction, 1);

		switch (instruction)
		{
		case 0: // Ping
			spi_read_blocking(spi0, 0, &data, 1);
			spi_write_blocking(spi0, &data, 1);
			break;

		case 1: // Malloc
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

	}

    return 0;
}