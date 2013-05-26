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

#define _POSIX_C_SOURCE 199309L
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
#include <syslog.h>
#include "../sansgrid_router.h"
/** \file */


#define KHZ(freq) (1000*freq)
#define MHZ(freq) (1000*KHZ(freq))

/// SPI Clock Speed, in KHz
//#define SPI_SPEED_KHZ	512
//#define SPI_SPEED_KHZ	4000L
#define SPI_SPEED_KHZ	15
/**
 * \brief Write Cycle Time, in us
 *
 * After transferring WRITE_MAX_BYTES,
 * wait for this period of time before
 * continuing.
 */
#define WRITE_CYCLE_U	6000
/// The max number of bytes we can write before cycling
#define WRITE_MAX_BYTES 100

/**
 * \brief What pin the slave interrupt is on
 * 
 * Uses wiringPi pin number 
 */
#define SLAVE_INT_PIN 	2

static sem_t wait_on_slave;
static int sem_initd = 0;
static int g_fd = 0;
static pthread_mutex_t transfer_lock;

/**
 * \brief Interrupt handler: waits until slave wants to send data
 *
 * An interrupt is set up in sgSerialReceive on a GPIO pin.
 * When that pin is asserted, this function allows a read transaction
 * to commence.
 */
void sgSerialSlaveSending(void) {
	printf("Got an interrupt\n");
	sem_post(&wait_on_slave);
}


static SansgridSerial *sgSerialCP(SansgridSerial *sg_serial) {
	// allocate a new structure and copy sg_serial into it
	SansgridSerial *sg_serial_cpy;
	sg_serial_cpy = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(sg_serial_cpy, sg_serial, sizeof(SansgridSerial));
	return sg_serial_cpy;
}



/**
 * \brief Prepare for impending SPI Transaction
 *
 * Called before every SPI transaction.
 * \returns
 * On success, a file descriptor is returned. You can write to
 * the file descriptor using Linux syscalls read and write. \n
 * You do have to close the file descriptor using the close() syscall
 * once you are done.
 */
int spiSetup(void) {
	FILE *FPTR;
	FPTR = popen("gpio load spi", "r");
	fclose(FPTR);
	// FIXME: use SLAVE_INT_PIN here
	FPTR = popen("gpio export 2 in", "r");
	fclose(FPTR);
	//syslog(LOG_DEBUG, "Using pin %i", SLAVE_INT_PIN);
	if (wiringPiSetupSys() == -1) {
		syslog(LOG_ERR, "Couldn't setup wiringPi system!");
		exit(EXIT_FAILURE);
	}
	if (wiringPiISR(SLAVE_INT_PIN, INT_EDGE_FALLING, &sgSerialSlaveSending) < 0) {
		syslog(LOG_ERR, "Couldn't setup interrupt on pin!");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_init(&transfer_lock, NULL);
	return 0;
}


/**
 * \brief Transfer data full-duplex over SPI
 *
 * Transfer raw data full-duplex over a wire.
 * \param[in,out] buffer	Data to send. Received data is returned here.
 * \param 		size		Size of data to send.
 * \returns
 * Data is returned in the buffer. \n
 * return 0 always.
 */
int spiTransfer(char *buffer, int size) {
	// only a certain amount of byte can be written at a time. see below
	int bounded_size = (size > WRITE_MAX_BYTES ? WRITE_MAX_BYTES : size);
	struct timespec req = { 0, WRITE_CYCLE_U*1000L };
	struct timespec rem;

	wiringPiSPIDataRW(0, (unsigned char*)buffer, bounded_size);
	// Wait for the write to cycle
	//usleep(WRITE_CYCLE_U);
	nanosleep(&req, &rem);
	if (size > WRITE_MAX_BYTES) {
		// Only a certain amount of bytes can be writeen at a time
		// If we go over that limit, break the write up into multiple
		// chunks
		spiTransfer(&buffer[WRITE_MAX_BYTES], size-WRITE_MAX_BYTES);
	}

	return 0;
}

int8_t sgSerialOpen(void) {
	// Set up SPI
	if ((g_fd = wiringPiSPISetup (0, KHZ(SPI_SPEED_KHZ))) < 0) {
		syslog(LOG_ERR, "SPI Setup failed: %s\n", strerror (errno));
		return -1;
	} else {
		return 0;
	}
	return 0;
}

int spiOpen(void) {
	// Wrapper to setup SPI
	int8_t exit_code;
	if ((exit_code = sgSerialOpen()) < 0) {
		return exit_code;
	} else {
		return g_fd;
	}
}

/**
 * \brief Send data over SPI
 *
 * The SansgridSerial structure is converted into raw bytes
 * and transferred over a serial wire. 
 * \param sg_serial[in]		Data to be sent. Contains a Sansgrid Payload
 * \param size[in]			Size of data. Not currently used.
 */
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	int fd;
	char buffer[size+1];
	int8_t exit_code = 0;
	pthread_mutex_lock(&transfer_lock);

	syslog(LOG_INFO, "Sending data over Serial");

	if ((fd = spiOpen()) == -1) {
		return -1;
	}

	memcpy(buffer, sg_serial, size);
	spiTransfer(buffer, size);

	close(fd);
	memcpy(sg_serial, buffer, size);
	pthread_mutex_unlock(&transfer_lock);
	if (buffer[0] == SG_SERIAL_CTRL_VALID_DATA)
		queueEnqueue(dispatch, sgSerialCP(sg_serial));
	

	return exit_code;
}


/**
 * \brief Receive data over SPI
 *
 * Data is received over serial wire and then converted
 * into a SansgridSerial structure.  
 * \param sg_serial[out]		Where received data is placed. 
 * \param size[out]				Size of the returned payload
 */
int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	// Code from
	// https://git.drogon.net/?p=wiringPi;a=blob;f=examples/isr.c;h=2bef54af13a60b95ad87fbfc67d2961722eb016e;hb=HEAD
	char buffer[sizeof(SansgridSerial)+1];
	syslog(LOG_INFO, "Waiting for data over serial");
	if (!sem_initd) {
		sem_init(&wait_on_slave, 0, 0);
		sem_initd = 1;
	}
	memset(buffer, 0x0, sizeof(buffer));
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memset(*sg_serial, 0x0, sizeof(SansgridSerial));
	buffer[0] = SG_SERIAL_CTRL_NO_DATA;

	// receive data
	while (sem_trywait(&wait_on_slave) == 0);
	if (digitalRead((SLAVE_INT_PIN)))
		sem_wait(&wait_on_slave);

	// Slave wants to send data
	/*
	if ((fd = spiOpen()) == -1) {
		syslog(LOG_ERR, "setting up SPI failed");
		exit(EXIT_FAILURE);
	}
	spiTransfer(buffer, sizeof(SansgridSerial));
	close(fd);
	*/
	sgSerialSend(*sg_serial, sizeof(SansgridSerial));
	memcpy(buffer, *sg_serial, sizeof(SansgridSerial));
	if (buffer[0] != SG_SERIAL_CTRL_VALID_DATA) {
		syslog(LOG_WARNING, "Bad data on SPI");
		sleep(1);
		return -1;
	}

	//memcpy(*sg_serial, buffer, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);

	return 0;
}

//#else
//#error "Expected Raspberry Pi Architecture Definition"
//#endif // SG_ARCH_PI

// vim: ft=c ts=4 noet sw=4:


