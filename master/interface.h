#pragma once

#include "hardware/spi.h"

struct sub_pico_interface {
	spi_inst_t *spi;
	uint pins[4];
	uint32_t baud;
};

typedef struct sub_pico_interface interface;

void initSPI(interface i);

uint32_t pingSub(interface connection, uint8_t pingData);

int resetSub(interface connection);

uint8_t *mallocSub(interface connection, size_t size);