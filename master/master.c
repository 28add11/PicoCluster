/*
//	Code for "Master" pico in cluster
//	
//	Made by Nicholas West, 2024
*/

#include "pico/stdlib.h"
#include "pico/time.h"
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

	spi_init(spi0, 100000);

	spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);

	gpio_set_function(4, GPIO_FUNC_SPI);
	gpio_set_function(5, GPIO_FUNC_SPI);
	gpio_set_function(6, GPIO_FUNC_SPI);
	gpio_set_function(7, GPIO_FUNC_SPI);

	uint8_t returnDat; 
	uint8_t instrDat[2];

	uint8_t mallocSize[sizeof(size_t)];

	mallocSize[sizeof(size_t) - 1] = 255;

	// Reset slave pico to synch signals
	instrDat[0] = 1;
	instrDat[1] = 0;
	spi_write_blocking(spi0, instrDat, 2); // send reset signal
	sleep_ms(15); // Give time for reset signal to work

	while(1) {
		
		// Send a test ping
		uint32_t startTime = time_us_32();

		instrDat[0] = 2; // Ping instruction
		instrDat[1] = 0xA5; // Ping data (A5 because I like it for testing the interface)

		spi_write_blocking(spi0, instrDat, 2);
		sleep_us(100);
		spi_read_blocking(spi0, 0, &returnDat, 1);
		if (returnDat != instrDat[1]) {
			printf("Error: ping doesn't match\nGot back %i instead\n", returnDat);
		}

		uint32_t endTime = time_us_32() - startTime;
		printf("Ping time: \t%i\n", endTime);
		instrDat[1]++;

		/*
		// Now we test the malloc
		instrDat[0] = 3; // malloc instruction

		spi_write_blocking(spi0, instrDat, 1); // Sending the instruction alone because size_t is funny
		spi_write_blocking(spi0, mallocSize, sizeof(size_t)); // Size to allocate

		uint8_t *returnedPoint;
		spi_read_blocking(spi0, 0, (uint8_t *)(&returnedPoint), sizeof(uint8_t *));
		printf("Pointer result: \t%p\n\n", returnedPoint);
		*/

		sleep_ms(100);
	}

    return 0;
}