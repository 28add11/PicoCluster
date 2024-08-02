/*
//	Code for "Master" pico in cluster
//	
//	Made by Nicholas West, 2024
*/

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/rand.h"
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


	uint8_t instrDat[2];

	uint8_t returnDat; 

	size_t mallocSize = 256;

	uint8_t *dataAddr;

	uint32_t offChipDat[64];
	uint32_t result;

	uint8_t pingDat = 0;
	int allocated = 0;

	// Reset slave pico to synch signals
	instrDat[0] = 1;
	instrDat[1] = 0;
	spi_write_blocking(spi0, instrDat, 2); // send reset signal
	sleep_ms(15); // Give time for reset signal to work

	while(1) {
		
		// Send a test ping
		uint32_t startTime = time_us_32();

		instrDat[0] = 2; // Ping instruction
		instrDat[1] = pingDat; // Ping data (A5 because I like it for testing the interface)

		spi_write_blocking(spi0, instrDat, 2);
		sleep_us(100); // Processing delay
		spi_read_blocking(spi0, 0, &returnDat, 1);
		if (returnDat != pingDat) {
			printf("Error: ping doesn't match\nGot back %i instead\n", returnDat);
		}

		uint32_t endTime = time_us_32() - startTime;
		printf("Ping time: \t%i\n", endTime);
		pingDat++;


		// Now we test the malloc
		instrDat[0] = 3; // mem access instruction
		instrDat[1] = 2; // malloc sub instruction

		spi_write_blocking(spi0, instrDat, 2); // Sending the instruction
		sleep_us(100);

		spi_write_blocking(spi0, (uint8_t *)(&mallocSize), sizeof(size_t)); // Size to allocate
		sleep_ms(3); // Processing delay
		spi_read_blocking(spi0, 0, (uint8_t *)(&dataAddr), sizeof(uint8_t *));
		if (dataAddr == NULL) {
			printf("WTF WHY NULL\n\n");
			return 1;
		}
		allocated += 256;

		// Fill off-pico mem with random data
		for (int i = 0; i < 64; i++) {
			result = 0;
			offChipDat[i] = get_rand_32();

			instrDat[1] = 1; // write sub instruction
			// Send to slave
			// TODO: make this system less shit
			for (int j = 0; j < sizeof(uint32_t); j++) { // Loop for every address
				spi_write_blocking(spi0, instrDat, 2); // Sending the instruction
				sleep_us(100);
				uint32_t *address = (uint32_t *)dataAddr + i;
        		uint8_t *byteAddress = (uint8_t*)address + j;
				spi_write_blocking(spi0, (uint8_t *)(&byteAddress), sizeof(uint32_t)); // send adress of byte
				sleep_us(100);
				uint8_t sendData = (offChipDat[i] >> (j * 8));
				spi_write_blocking(spi0, &sendData, 1); // send byte, lsb first because little endian is cool
				sleep_ms(3); // Even though this should be pretty fast im not gonna test it
			}

			instrDat[1] = 0; // read sub instruction

			for (int j = 0; j < sizeof(uint32_t); j++) {
				spi_write_blocking(spi0, instrDat, 2); // Sending the instruction
				sleep_us(100);
				uint32_t *address = (uint32_t *)dataAddr + i;
        		uint8_t *byteAddress = (uint8_t*)address + j;
				spi_write_blocking(spi0, (uint8_t *)(&byteAddress), sizeof(uint32_t)); // send adress of byte
				//spi_write_blocking(spi0, (uint8_t *)(&dataAddr[j + i * sizeof(uint32_t)]), sizeof(uint32_t)); // send adress of byte
				sleep_ms(2);
				spi_read_blocking(spi0, 0, &returnDat, 1); // Read the byte
				result = result | (((uint32_t)(returnDat)) << (j * 8)); // Little endian
			}

			if (result != offChipDat[i]) {
				printf("Error: Value %i at index %i does not equal off chip value %i\n\n", offChipDat[i], i, result);
				break;
			}
			sleep_ms(2);
		}

		if (allocated >= 200000) {
			printf("All good!\n\n");
			break;
		}
		
		printf("Data good through %i bytes\n", allocated);
		
		sleep_ms(10);
	}

    return 0;
}