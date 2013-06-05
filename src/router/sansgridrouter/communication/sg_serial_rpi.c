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

/// Required for nanosleep
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
#include "../dispatch/dispatch.h"
/** \file */


/// Convert KHz to Hz
#define KHZ(freq) (1000*freq)
/// Convert MHz to Hz
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

/// SPI Transaction indicator
static sem_t wait_on_slave;				// SPI transaction indicator
/// Make sure semaphore wait_on_slave is initialized
static int sem_initd = 0;				// flag if semaphore is initialized
/// Global file descriptor for SPI transfer
static int g_fd = 0;					// global file descriptor
/// SPI atomic lock
static pthread_mutex_t transfer_lock;	// atomic lock
/// Transmission buffer
static Queue *tx_buffer = NULL;			// transfer buffer


/**
 * Print a SansgridSerial structure
 */
static void spiPrintSgSerial(char *buffer, int size) {
	// print an sg_serial structure stored in a buffer
	printf("control byte: %.2x\n", buffer[0]);
	printf("IP address: ");
	for (uint32_t i=1; i<16+1; i++)
	    printf("%.2x", buffer[i]);
	printf("\ndata: ");
	for (uint32_t i=17; i<size; i++)
	    printf("%.2x", buffer[i]);
	printf("\n");
}


/**
 * \brief Interrupt handler: waits until slave wants to send data
 *
 * An interrupt is set up in sgSerialReceive on a GPIO pin.
 * When that pin is asserted, this function allows a read transaction
 * to commence.
 */
void sgSerialSlaveSending(void) {
	// Interrupt handler for sgSerialReceive
	printf("Got an interrupt\n");
	sem_post(&wait_on_slave);
}


/**
 * \brief Create a copy of a SansgridSerial payload
 * \param[in]	sg_serial	SansgridSerial structure to copy
 * \returns
 * A new chunk of memory is allocated on the heap and sg_serial is
 * copied into the new chunk. This chunk is returned.
 */
static SansgridSerial *sgSerialCP(SansgridSerial *sg_serial) {
	// allocate a new structure and copy sg_serial into it
	SansgridSerial *sg_serial_cpy;
	sg_serial_cpy = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(sg_serial_cpy, sg_serial, sizeof(SansgridSerial));
	return sg_serial_cpy;
}



/**
 * \brief Setup resources for SPI transactions
 *
 * Called Once to setup buffers, locks, and GPIO
 * \returns
 * If Setup has been called recently, 1 is returned as a warning. \n
 * If GPIO can't be loaded, the entire system is stopped with EXIT_FAILURE. \n
 * Otherwise, 0 is returned.
 */
int spiSetup(void) {
	// Setup resources for SPI transactions
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
	if (tx_buffer == NULL) {
		tx_buffer = queueInit(100);
	} else {
		syslog(LOG_WARNING, "spiSetup has already been called!");
		return 1;
	}
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


/**
 * \brief Prepare for SPI transfer (API)
 *
 * Note that to use this, you must have an int g_fd file descriptor
 * that this can write to. You must close that file descriptor when
 * transfer is done. You should use spiOpen(), which is a wrapper
 * for this function that returns the file descriptor directly.
 * \returns
 * This function returns 0 on success, and -1 on failure. \n
 * A global file descriptor is stored in the int g_fd field.
 */
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

/**
 * \brief Prepare for SPI transfer (Wrapper)
 *
 * A wrapper for sgSerialOpen to account for the int8_t return
 * for the API call, which cannot pass back a file descriptor.
 * This wrapper function solves that.
 * \returns
 * This function returns a file descriptor that can be used for 
 * writing to or reading from SPI. It must be closed once the
 * transfer is complete.
 */
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
 * \brief Enqueue data to be sent over SPI
 *
 * This function gets data ready to be sent over SPI.
 * Note that the actual transfer is done in sgSerialReceive with
 * the current scheme.
 * \param sg_serial[in]		Data to be sent. Contains a Sansgrid Payload
 * \param size[in]			Size of data. Not currently used.
 */
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	SansgridSerial *sg_serial_cp = NULL;

	sg_serial_cp = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(sg_serial_cp, sg_serial, sizeof(SansgridSerial));
	queueEnqueue(tx_buffer, sg_serial_cp);
	sem_post(&wait_on_slave);
	return 0;
}


/**
 * \brief Receive data full-duplex over SPI
 *
 * If there is data waiting to be sent, that is transferred.
 * Data is also received over serial wire and then converted
 * into a SansgridSerial structure.  
 * \param sg_serial[out]		Where received data is placed. 
 * \param size[out]				Size of the returned payload, zero if not packet
 * received
 */
int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Transmit/Receive serialdata, size of packet stored in size
	// Code from
	// https://git.drogon.net/?p=wiringPi;a=blob;f=examples/isr.c;h=2bef54af13a60b95ad87fbfc67d2961722eb016e;hb=HEAD
	int i;
	SansgridSerial *sg_serial_out = NULL;
	char buffer[sizeof(SansgridSerial)+1];
	int fd;
	// Set when we're sending data
	int sending = 0;
	int exit_code = 0;
	*size = sizeof(SansgridSerial);
	syslog(LOG_INFO, "Waiting for data over serial");
	if (!sem_initd) {
		sem_init(&wait_on_slave, 0, 0);
		sem_initd = 1;
	}
	memset(buffer, 0x0, sizeof(buffer));
	buffer[0] = SG_SERIAL_CTRL_NO_DATA;

	// receive data
	
	// first make sure semaphore is zeroed out
	while (sem_trywait(&wait_on_slave) == 0);
	// If there are no pending requests, wait until we get one
	if (digitalRead((SLAVE_INT_PIN)) && !queueSize(tx_buffer))
		sem_wait(&wait_on_slave);

	// Transfer about to occur
	if (queueTryDequeue(tx_buffer, (void**)&sg_serial_out) >= 0) {
		// Data needs to be sent
		sg_serial_out->control = SG_SERIAL_CTRL_VALID_DATA;
		*sg_serial = sg_serial_out;
		memcpy(buffer, sg_serial_out, *size);
		sending = 1;
	} else {
		// No data needs to be sent
		*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
		(*sg_serial)->control = SG_SERIAL_CTRL_NO_DATA;
		sending = 0;
	}


	pthread_mutex_lock(&transfer_lock);
	// LOCKED


	// Make sure only one thread can access SPI at a time
	syslog(LOG_INFO, "Sending data over Serial");

	// this needs to be atomic, since fd comes from static global g_fd
	if ((fd = spiOpen()) == -1) {
		return -1;
	}

	// (debug) Print our data to send 
	printf("Sending:\n");
	spiPrintSgSerial(buffer, *size);

	spiTransfer(buffer, *size);
	close(fd);

	// (debug) Print what we got 
	printf("Receiving:\n");
	spiPrintSgSerial(buffer, *size);

	memcpy(*sg_serial, buffer, *size);
	if (buffer[0] == SG_SERIAL_CTRL_VALID_DATA) {
		queueEnqueue(dispatch, sgSerialCP(*sg_serial));
		*size = sizeof(SansgridSerial);
		// Radio can't get spammed. Throttle sending
		exit_code = 0;
	} else {
		// No need for sg_serial anymore
		//free(sg_serial);
		//*size = 0;
		if (buffer[0] != SG_SERIAL_CTRL_VALID_DATA
				&& sending == 0) {
			// Didn't send valid data, and didn't get valid data
			syslog(LOG_WARNING, "Bad data on SPI");
			exit_code = -1;
		}
	}

	for (i=0; i<3; i++) {
		sleep(1);
		if (!digitalRead(SLAVE_INT_PIN))
			break;
	}


	pthread_mutex_unlock(&transfer_lock);
	// UNLOCKED



	return exit_code;
}


/**
 * \brief Teardown dynamic resources for SPI transactions, leave GPIO state
 *
 * Undoes some of what spiSetup() does. \n
 * GPIO pins are unexported, buffers are destroyed.
 * \returns
 * On success, 0 is returned. \n
 * If buffers are already NULL, a warning is returned.
 */
int spiTeardown(void) {
	// Teardown resources for SPI transactions
	if (tx_buffer == NULL) {
		syslog(LOG_WARNING, "Trying to free NULL tx_buffer");
		return 1;
	}
	pthread_mutex_destroy(&transfer_lock);
	free(tx_buffer);
	tx_buffer = NULL;
	return 0;
}


//#else
//#error "Expected Raspberry Pi Architecture Definition"
//#endif // SG_ARCH_PI

// vim: ft=c ts=4 noet sw=4:


