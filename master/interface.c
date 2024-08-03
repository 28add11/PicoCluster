#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include <stdio.h>
#include <stdlib.h>

#include "interface.h"


void initSPI(interface connection) {
	/*
	Initializes a connection, handling pins and spi settings
	*/

	spi_init(connection.spi, connection.baud);
	spi_set_format(connection.spi, 8, 0, 0, SPI_MSB_FIRST);

	for (int i = 0; i < 4; i++) {
		gpio_set_function(connection.pins[i], GPIO_FUNC_SPI);
	}
}


uint32_t pingSub(interface connection, uint8_t pingData) {
	/*
	Pings the sub device, returning the round trip time for the ping, or 0 if the reply was wrong
	*/

	uint8_t instrDat[2] = {2, pingData}; // 2 is ping instruction
	uint8_t reply;

	uint32_t startTime = time_us_32();

	spi_write_blocking(connection.spi, instrDat, 2);
	sleep_us(100); // Processing delay
	spi_read_blocking(connection.spi, 0, &reply, 1);

	uint32_t endTime = time_us_32() - startTime;

	return (reply == pingData ? endTime : 0);
}


int resetSub(interface connection) {
	/*
	Send an instruction to reset the sub device's interface to sync
	Also sends a ping to check for integrity, returns 1 if ping does not match
	*/

	uint8_t instrDat[2] = {1, 0}; // 1 is reset instruction
	spi_write_blocking(connection.spi, instrDat, 2);
	sleep_ms(15); // Give time for reset signal to work

	return !pingSub(connection, 0xA5);
}

uint8_t *mallocSub(interface connection, size_t size) {
	/*
	allocates size of memory on the sub device, returning a pointer to that memory
	*/

	uint8_t instrDat[2] = {3, 2}; // 3 is mem interface instruction, 2 is malloc sub instruction
	uint8_t *addr;

	spi_write_blocking(connection.spi, instrDat, 2); // Sending the instruction
	sleep_us(100); // Proc delay
	spi_write_blocking(connection.spi, (uint8_t *)(&size), sizeof(size_t)); // Size to allocate
	sleep_ms(3); // malloc takes a rlly long time for some reason
	spi_read_blocking(connection.spi, 0, (uint8_t *)(&addr), sizeof(uint8_t *));

	return addr;
}

void writeSub(interface connection, uint8_t data, uint8_t *address) {
	/*
	Writes a byte to the sub device's memory
	*/

	uint8_t instrDat[2] = {3, 1}; // 3 is mem interface instruction, 1 is write sub instruction
	spi_write_blocking(connection.spi, instrDat, 2);
	sleep_us(100);
	spi_write_blocking(connection.spi, (uint8_t *)(&address), sizeof(uint8_t *)); // send adress of byte
	sleep_ms(1);
	spi_write_blocking(connection.spi, &data, 1);
	return;
}

uint8_t readSub(interface connection, uint8_t *address) {
	/*
	Reads data from the sub device, returning the data read
	*/
	
	uint8_t instrDat[2] = {3, 0}; // 3 is mem interface instruction, 0 is read sub instruction
	uint8_t data;
	spi_write_blocking(connection.spi, instrDat, 2);
	sleep_us(100);
	spi_write_blocking(connection.spi, (uint8_t *)(&address), sizeof(uint8_t *)); // send adress of byte
	sleep_ms(1);
	spi_read_blocking(connection.spi, 0, &data, 1); // Read the byte

	return data;
}

void writeSub32(interface connection, uint32_t data, uint32_t *address) {
	/*
	Sends a uint32_t to the sub device
	*/

	uint8_t *byteAddr;
	uint8_t byteData;

	for (int i = 0; i < sizeof(uint32_t); i++) {
		byteAddr = (uint8_t *)(address) + i;
		byteData = data >> (i * 8);
		writeSub(connection, byteData, byteAddr);
		sleep_us(100);
	}

	return;
}

uint32_t readSub32(interface connection, uint32_t *address) {
	/*
	Reads a uint32_t from sub device, returning it
	*/
	uint8_t *byteAddr;
	uint8_t byteData;
	uint32_t data;

	for (int i = 0; i < sizeof(uint32_t); i++) {
		byteAddr = (uint8_t *)(address) + i;
		byteData = readSub(connection, byteAddr);
		data = data | ((uint32_t)(byteData) << (i * 8));
		sleep_us(100);
	}

	return data;
}
