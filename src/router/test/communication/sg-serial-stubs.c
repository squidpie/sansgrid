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

static FILE *FPTR_SPI_WRITE = NULL,
			*FPTR_SPI_READ = NULL;


void sgSerialTestSetReader(FILE *FPTR) {
	FPTR_SPI_READ = FPTR;
}

void sgSerialTestSetWriter(FILE *FPTR) {
	FPTR_SPI_WRITE = FPTR;
}


int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	int i;
	SANSGRID_UNION(SansgridSerial, SGSU) sg_serial_union;


	if (FPTR_SPI_WRITE == NULL)
		return -1;

	sg_serial_union.formdata = sg_serial;
	
	for (i=0; i<sizeof(SansgridSerial); i++) {
		putc(sg_serial_union.serialdata[i], FPTR_SPI_WRITE);
	}
	
	return 0;
}



int8_t sgSerialReceive(SansgridSerial **sg_serial, uint32_t *size) {
	// Receive serialdata, size of packet stored in size
	int i;
	char lptr[sizeof(SansgridSerial)+1];

	if (FPTR_SPI_READ == NULL)
		return -1;

	// Read from the pipe 
	for (i=0; i<(sizeof(SansgridSerial)) && FPTR_SPI_READ; i++) {
		lptr[i] = fgetc(FPTR_SPI_READ);
		if (lptr[i] == EOF && (i == (sizeof(SansgridSerial)-1))) {
			//printf("(Serial) Dropping Packet\n");
			return 1;
		}
	}
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, lptr, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);
	return 0;
}



// vim: ft=c ts=4 noet sw=4:

