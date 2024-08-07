/*
//	Code for "Master" pico in cluster
//	
//	Made by Nicholas West, 2024
*/

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/rand.h"
#include "hardware/spi.h"
#include <stdio.h>
#include <stdlib.h>

#include "interface.h"

#include "blinkLED_bin.h" // Generated header from cmake


int main(void) {
    stdio_init_all();

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);

	printf("started\n");

	interface con0 = {spi0, {4, 5, 6, 7}, 500000};	

	initSPI(con0);


	uint32_t pingTime;

	uint8_t *dataAddr;

	uint32_t offChipDat[64];
	uint32_t result;

	uint8_t pingDat = 0;
	int allocated = 0;

	// Reset slave pico to synch signals
	int conStatus = resetSub(con0);
	if (conStatus) {
		printf("Could not connect to sub, exiting\n\n");
		return 1;
	}

	sleep_ms(1);

	// Attempt to execute program on sub

	uint8_t byteData;
	uint8_t *byteAddr;

	// Get our distance from where the program needs to be
	uint8_t *deltaAddr = mallocSub(con0, 1);
	uint32_t 

	dataAddr = mallocSub(con0, blinkLED_bin_len + 4); // +4 to satisfy arm's boundary requirement
	if (dataAddr == NULL) {
		printf("Memory allocation failed\n");
		return 1;
	}


	dataAddr = (uint8_t *)(((uintptr_t)dataAddr + 3) & ~3); // Arm boundary requirements

	printf("Allocated memory at aligned address: %p \t of size: %i\n", (void*)dataAddr, blinkLED_bin_len + 4);

	for (int i = 0; i < blinkLED_bin_len; i++) { // Upload the code
		byteAddr = dataAddr + i;
		byteData = blinkLED_bin[i];
		writeSub(con0, byteData, byteAddr);
		sleep_us(10);
	}

	// Verify that load was correct
	for (int i = 0; i < blinkLED_bin_len; i++) {
		byteAddr = dataAddr + i;
		byteData = readSub(con0, byteAddr);
		if (byteData != blinkLED_bin[i]) {
			printf("Load not correct, exiting.\n\n");
			gpio_put(LED_PIN, 1);
    		sleep_ms(250);
    		gpio_put(LED_PIN, 0);
			return 1;
		}
		
		sleep_us(10);
	}

	executeSub(con0, dataAddr);


	/*
	while(1) {
		
		// Send a test ping
		pingTime = pingSub(con0, pingDat);
		if (!pingTime) {
			printf("Error: ping doesn't match!\n");
		}

		printf("Ping time: \t%i\n", pingTime);
		pingDat++;


		// Now we test the malloc
		dataAddr = mallocSub(con0, 255);
		if (dataAddr == NULL) {
			printf("WTF WHY NULL\n\n");
			return 1;
		}
		allocated += 256;

		// Fill off-pico mem with random data
		for (int i = 0; i < 64; i++) {
			offChipDat[i] = get_rand_32();

			// Send to slave
			uint32_t *address = (uint32_t *)(dataAddr) + i;
			writeSub32(con0, offChipDat[i], address);

			sleep_us(100);
		}

		// Read from off chip mem and confirm
		for (int i = 0; i < 64; i++) {
			uint32_t *address = (uint32_t *)(dataAddr) + i;
        	result = readSub32(con0, address);

			if (result != offChipDat[i]) {
				printf("Error: Value %i at index %i does not equal off chip value %i\n\n", offChipDat[i], i, result);
				break;
			}
			sleep_us(100);
		}

		if (allocated >= 200000) {
			printf("All good!\n\n");
			break;
		}
		
		printf("Done through %i bytes\n", allocated);
		
		sleep_ms(10);
	}
	*/





    return 0;
}