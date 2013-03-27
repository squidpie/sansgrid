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
#include <semaphore.h>
#include <string.h>
#include "../../../sg_serial.h"
#include "../tests.h"

static FILE *FPTR_SPI_WRITE = NULL,
			*FPTR_SPI_READ = NULL;
static sem_t *SPI_READ = NULL;
static sem_t *spi_read_sync = NULL;
static sem_t *spi_write_sync = NULL;


void sgSerialTestSetReader(FILE *FPTR, sem_t *readlock) {
	FPTR_SPI_READ = FPTR;
}

void sgSerialTestSetWriter(FILE *FPTR) {
	FPTR_SPI_WRITE = FPTR;
}

void sgSerialTestSetReadlock(sem_t *readlock) {
	SPI_READ = readlock;
}

void sgSerialTestSetRWSync(sem_t *readlock, sem_t *writelock) {
	spi_read_sync = readlock;
	spi_write_sync = writelock;
}


int8_t sgSerialSend(SansgridSerial *sg_serial, uint32_t size) {
	// Send size bytes of serialdata
	int i;
	SANSGRID_UNION(SansgridSerial, SGSU) sg_serial_union;


	if (FPTR_SPI_WRITE == NULL)
		return -1;

	sg_serial_union.formdata = sg_serial;
	
	if (spi_write_sync)
		sem_post(spi_write_sync);
	if (spi_read_sync)
		sem_wait(spi_read_sync);
	for (i=0; i<sizeof(SansgridSerial) && FPTR_SPI_WRITE; i++) {
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
	if (spi_read_sync)
		sem_post(spi_read_sync);
	if (spi_write_sync)
		sem_wait(spi_write_sync);
	for (i=0; i<(sizeof(SansgridSerial)) && FPTR_SPI_READ; i++) {
		lptr[i] = fgetc(FPTR_SPI_READ);
	}
	if (SPI_READ) {
		if (sem_trywait(SPI_READ) == -1) {
			return 1;
		}
	}
	if (i < sizeof(SansgridSerial)) {
#if TESTS_DEBUG_LEVEL > 0
		printf("(Serial) Dropping Packet at %i of %i\n", i, 
				sizeof(SansgridSerial));
#endif
		return 1;
	}
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, lptr, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);
	return 0;
}



// vim: ft=c ts=4 noet sw=4:

