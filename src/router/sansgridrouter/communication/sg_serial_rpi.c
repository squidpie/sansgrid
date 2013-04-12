/* Definitions for communication functions
 * Specific to the Raspberry Pi Platform
 *
 * Copyright (C) 2013 SansGrid
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 */

#include <stdint.h>
#include "../../../sg_serial.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define KHZ(freq) (1000*freq)
#define MHZ(freq) (1000*KHZ(freq))

// EEPROM-specific defines
#define SPI_SPEED_MHZ	2
#define WRITE 		0x02
#define READ		0x03
#define WRITE_ENABLE 	0x06
#define WRITE_CYCLE	5000

int spiWrite(char *buffer, uint16_t address, int size) {
	int i;
	int fd;
	// only 32 bytes can be written at a time; see below
	int bounded_size = (size > 32 ? 32 : size);
	// prepend the command and address to the data
	char newbuffer[bounded_size+3];

	// Set up SPI
	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));

	// Have to make room for command and address
	for (i=3; i<bounded_size+3; i++)
		newbuffer[i] = buffer[i-3];

	// Allow writes to the EEPROM
	// Has to be done before every write cycle
	newbuffer[0] = WRITE_ENABLE;
	write(fd, newbuffer, 1);

	// Write to specified address
	newbuffer[0] = WRITE;
	newbuffer[1] = address >> 8;
	newbuffer[2] = address & 0xff;

	write(fd, newbuffer, bounded_size+3);
	// Wait for the write to cycle
	usleep(WRITE_CYCLE);
	close(fd);
	if (size > 32) {
		// Only one page (of 32 bytes) can be written
		// at a time. If more than 32 bytes are being written,
		// break the line into multiple pages
		spiWrite(&buffer[32], address+0x0020, size-32);
	}

	return 0;
}


int spiRead(char *buffer, uint16_t address, int size) {
	// Read from an EEPROM chip over SPI
	int fd;
	char newbuffer[size+3];
	int i;

	// Set up reading from EEPROM
	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));

	// Prepend command and address to buffer
	newbuffer[0] = READ;
	newbuffer[1] = address >> 8;
	newbuffer[2] = address & 0xff;

	// Send command/address, read data
	wiringPiSPIDataRW(0, newbuffer, size+3);

	// shift the command and address out of buffer
	for (i=0; i<size; i++)
		buffer[i] = newbuffer[i+3];
	// finish up
	close(fd);

	return 0;
}

int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	return -1;
}

int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	return -1;
}

/*
// EEPROM-Specific main function
int main(void) {
	int i;
	int fd;
	char buffer[70];

	// Writing to the EEPROM chip
	snprintf(buffer, 64, "This is a much longer sentence. It is bigger than 32.");
	spiWrite(buffer, 0x0000, 64);

	snprintf(buffer, 32, "Well I'll be!");
	spiWrite(buffer, 0x0040, 32);


	// Read using wraparound
	spiRead(buffer, 0x0000, 64);
	printf("%s\n", buffer);

	// Read the second datum
	spiRead(buffer, 0x0040, 32);
	printf("%s\n", buffer);

	return 0;
}
*/


// vim: ft=c ts=4 noet sw=4:


