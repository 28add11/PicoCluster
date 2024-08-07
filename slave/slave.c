/*
//	Code for "Slave" pico in cluster
//
//	Written by Nicholas West, 2024
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/resets.h"
//#include "hbi/hbi.h"
#include <stdio.h>
#include <stdlib.h>

#include "slave.h"

int main(void) {
    stdio_init_all();

	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);

	printf("started\n");

	spi_init(spi0, 500000);

	spi_set_slave(spi0, true);

	spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);
	
	gpio_set_function(4, GPIO_FUNC_SPI);
	gpio_set_function(5, GPIO_FUNC_SPI);
	gpio_set_function(6, GPIO_FUNC_SPI);
	gpio_set_function(7, GPIO_FUNC_SPI);

	gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
	gpio_put(21, 0);

	uint8_t value[2];

	

	while (1) {

		gpio_put(21, 0);
		
		spi_read_blocking(spi0, 0xFF, value, 2);

		//printf("Instruction:\t%i, %i\n", value[0], value[1]);

		switch (value[0]) {

		case 1: // Reset (only reset SPI block because we are stateless beond single instructions)
			
			reset_block(RESETS_RESET_SPI0_BITS); // Actually reset the block
			gpio_put(LED_PIN, 1);
			unreset_block_wait(RESETS_RESET_SPI0_BITS); // Also serves as a delay so LED can actually do stuff
			gpio_put(LED_PIN, 0);
			gpio_put(21, 1);

			spi_init(spi0, 500000);
			spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);
			spi_set_slave(spi0, true);
			break;
			
		case 2: // Ping
			gpio_put(21, 1);
			spi_write_blocking(spi0, &value[1], 1);
			break;

		case 3: // Memory access
			;
			size_t size;
			uint8_t *address;
			uint8_t data;

			switch (value[1]) { // Get what we are actually doing
			case 0: // Read
				// Get adress to read from
				gpio_put(21, 1);
				spi_read_blocking(spi0, 0xFF, (uint8_t *)(&address), sizeof(uint8_t *));

				data = *address;
				
				spi_write_blocking(spi0, &data, 1);
				break;

			case 1: // Write
				// Get adress to write to
				gpio_put(21, 1);
				spi_read_blocking(spi0, 0xFF, (uint8_t *)(&address), sizeof(uint8_t *));
				
				
				spi_read_blocking(spi0, 0xFF, &data, 1);

				*address = data;
				break;

			case 2: // Malloc
				// Get the size of the buffer to allocate
				gpio_put(21, 1);
				spi_read_blocking(spi0, 0xFF, (uint8_t *)(&size), sizeof(size_t));

				address = malloc(size);
				
				spi_write_blocking(spi0, (uint8_t *)(&address), sizeof(uint8_t *));
				break;
			
			default:
				break;
			}
			break;
		
		case 4: // Program context switch
			;
			// Get address of where to start running (program type defined in slave.h)
			program executable;
			spi_read_blocking(spi0, 0xFF, (uint8_t *)(&executable), sizeof(program));

			printf("Balling: starting execution\n\n");

			// Transfer execution
			executable = (program)((uint32_t)executable | 1); // | 1 relates to ARM's thumb execution, which is required for execution to work
			executable();
			break;

		default:
			break;
		}
	}

    return 0;
}