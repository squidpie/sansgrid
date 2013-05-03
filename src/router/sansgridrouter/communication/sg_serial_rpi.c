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

//#ifdef SG_ARCH_PI

#include <stdint.h>
#include <sgSerial.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>


#define KHZ(freq) (1000*freq)
#define MHZ(freq) (1000*KHZ(freq))

// EEPROM-specific defines
#define SPI_SPEED_KHZ	500
#define WRITE_CYCLE		1
#define WRITE_MAX_BYTES 1


ssize_t write_noconst(int fd, void *buf, size_t count)
{
	return write(fd, buf, count);
}


int spiTransfer(char *buffer, int size, ssize_t(*op)(int, void*, size_t))
{
	int i;
	int fd;
	// only 32 bytes can be written at a time; see below
	int bounded_size = (size > WRITE_MAX_BYTES ? WRITE_MAX_BYTES : size);
	// prepend the command and address to the data
	//char newbuffer[bounded_size];



	wiringPiSPIDataRW(0, buffer, bounded_size);
	// Wait for the write to cycle
	usleep(WRITE_CYCLE);
	if (size > WRITE_MAX_BYTES) {
		// Only one page (of 32 bytes) can be written
		// at a time. If more than 32 bytes are being written,
		// break the line into multiple pages
		spiTransfer(&buffer[WRITE_MAX_BYTES], size-WRITE_MAX_BYTES, op);
	}

	return 0;
}



int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	int fd;
	char buffer[size+1];

	// Set up SPI
	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));

	memcpy(buffer, sg_serial, size);
	spiTransfer(buffer, size, write_noconst);

	close(fd);

	return 0;
}


int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	// TODO: Get code from
	// https://git.drogon.net/?p=wiringPi;a=blob;f=examples/isr.c;h=2bef54af13a60b95ad87fbfc67d2961722eb016e;hb=HEAD
	return -1;
}

//#else
//#error "Expected Raspberry Pi Architecture Definition"
//#endif // SG_ARCH_PI

// vim: ft=c ts=4 noet sw=4:


