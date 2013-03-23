/* serial read/write stubs
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include "../../../sg_serial.h"

static FILE *FPTR_WRITE = NULL,
			*FPTR_READ = NULL;


void sgSerialTestSetReader(FILE *FPTR) {
	FPTR_READ = FPTR;
}

void sgSerialTestSetWriter(FILE *FPTR) {
	FPTR_WRITE = FPTR;
}



int8_t sgSerialSend(uint8_t *serial_data, uint32_t size) {
	// Send size bytes of serialdata
	int i;


	if (FPTR_WRITE == NULL)
		return -1;
	
	for (i=0; i<80; i++) {
		putc(serial_data[i], FPTR_WRITE);
	}
	
	return 0;
}



int8_t sgSerialReceive(uint8_t **serial_data, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	int timeout = 0;
	char lptr[81];

	if (FPTR_READ == NULL)
		return -1;

	// Read from the pipe 
	while (fgets(lptr, 81, FPTR_READ) == NULL) {
		timeout++;
		if (timeout > 10000) {
			return -1;
		}
		sched_yield();
	}
	*serial_data = (uint8_t*)malloc(80*sizeof(uint8_t));
	memcpy(*serial_data, lptr, 80);
	*size = 80;
	return 0;
}



// vim: ft=c ts=4 noet sw=4:

