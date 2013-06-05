/* Definitions for communication functions
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

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <sgSerial.h>
/** \file */


/// This is a Stub. Normally used to init SPI
int spiSetup(void) {
	// Stub. Does nothing
	return 0;
}

/// This is a Stub. Normally used to teardown SPI
int spiTeardown(void) {
	// Stub. Does nothing
	return 0;
}

/**
 * This is a Stub. \n
 * The SansgridSerial structure is normally converted into raw bytes
 * and transferred over a serial wire. But if we are on an
 * unsupported device, this stub is compiled in instead.
 * \param sg_serial[in]		Data to be sent. Contains a Sansgrid Payload
 * \param size[in]			Size of data. Not currently used.
 */
int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// (Stub) Send size bytes of serialdata
	if (sg_serial->payload[0] != SG_FLY)
		syslog(LOG_INFO, "Sending packet over SPI");

	size = -1;
	return size;
}


/**
 * This is a Stub. \n
 * Data is normally received over serial wire and then converted
 * into a SansgridSerial structure. But if we are on an unsupported
 * device, this stub is compiled in instead.
 * \param sg_serial[out]		Where received data is placed. 
 * \param size[out]			Size of the returned payload
 */
int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// (Stub) Receive serialdata, size of packet stored in size
	sem_t blocker;
	sem_init(&blocker, 0, 0);

	syslog(LOG_INFO, "Generic serial Receive: Waiting forever");
	sem_wait(&blocker);

	sem_destroy(&blocker);
	sg_serial = NULL;
	*size = -1;

	return *size;
}


// vim: ft=c ts=4 noet sw=4:

