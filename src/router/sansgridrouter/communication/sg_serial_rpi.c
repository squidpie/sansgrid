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

#include <pthread.h>
#include <semaphore.h>
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



int spiTransfer(char *buffer, int size)
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
		spiTransfer(&buffer[WRITE_MAX_BYTES], size-WRITE_MAX_BYTES);
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
	spiTransfer(buffer, size);

	close(fd);

	return 0;
}


int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	// Code from
	// https://git.drogon.net/?p=wiringPi;a=blob;f=examples/isr.c;h=2bef54af13a60b95ad87fbfc67d2961722eb016e;hb=HEAD
	char buffer[sizeof(SansgridSerial)+1];
	sem_t wait_on_slave;
	if (wiringPiSPISetup() < 0) {
		syslog(LOG_ERR, "Couldn't setup wiringPi for listening!");
		return -1;
	} 
	if (wiringPiISR(BUTTON_PIN, INT_EDGE_FALLING, &sgSerialSlaveSending) < 0) {
		syslog(LOG_ERR, "Couldn't setup interrupt on pin!");
		return -1;
	}
	sem_init(&wait_on_slave, 0, 0);
	memset(buffer, 0x0, sizeof(buffer));
	buffer[0] = SG_SERIAL_CTRL_NO_DATA;

	// receive data
	sem_wait(&wait_on_slave);
	// Slave wants to send data
	spiTransfer(buffer, sizeof(SansgridSerial));

	memcpy(*sg_serial, buffer, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);

	return 0;
}

//#else
//#error "Expected Raspberry Pi Architecture Definition"
//#endif // SG_ARCH_PI

// vim: ft=c ts=4 noet sw=4:


