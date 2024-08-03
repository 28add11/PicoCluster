#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include <stdio.h>
#include <stdlib.h>

#include "interface.h"


int initSPI(interface i) {


	spi_init(i.spi, i.baud);
	spi_set_format(i.spi, 8, 0, 0, SPI_MSB_FIRST);
}