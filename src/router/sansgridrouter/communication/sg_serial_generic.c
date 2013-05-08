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

int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	if (sg_serial->payload[0] != SG_FLY)
		syslog(LOG_INFO, "Sending packet over SPI");
	/*
	printf("sending");
	printf(" %2x | ", sg_serial->control);
	for (uint32_t i=0; i<IP_SIZE; i++) {
		printf("%.2x", sg_serial->ip_addr[i]);
	}
	printf(" | ");
	for (uint32_t i=0; i<PAYLOAD_SIZE; i++) {
		printf("%.2x", sg_serial->payload[i]);
	}
	printf("\n");
	*/

	return -1;
}

int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	sem_t blocker;
	sem_init(&blocker, 0, 0);
	sem_wait(&blocker);

	sem_destroy(&blocker);

	return -1;
}


// vim: ft=c ts=4 noet sw=4:

