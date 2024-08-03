#pragma once

#include "hardware/spi.h"

struct sub_pico_interface {
	spi_inst_t *spi;
	uint8_t pins[4];
	uint32_t baud;
};

typedef struct sub_pico_interface interface;

int initSPI(interface i);
