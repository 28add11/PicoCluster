#pragma once

#include "hardware/spi.h"

struct sub_pico_interface {
	spi_inst_t *spi;
	uint pins[4];
	uint32_t baud;
};

typedef struct sub_pico_interface interface;
typedef void (*program)(void);

void initSPI(interface i);

uint32_t pingSub(interface connection, uint8_t pingData);

int resetSub(interface connection);

uint8_t *mallocSub(interface connection, size_t size);

void writeSub(interface connection, uint8_t data, uint8_t *address);

uint8_t readSub(interface connection, uint8_t *address);

void writeSub32(interface connection, uint32_t data, uint32_t *address);

uint32_t readSub32(interface connection, uint32_t *address);

void executeSub(interface connection, uint8_t *executeable);
