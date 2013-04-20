/* read/write stubs
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
#define _POSIX_C_SOURCE 200809L          // Required for nanosleep()

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "../tests.h"
#include <sgSerial.h>
#include "sg_communication_stubs.h"

/*
 * Generic Send/Receive functions.
 * These hook into both serial and TCP stubs
 */
int8_t unixpipeSend(SansgridSerial *sg_serial, uint32_t size, TalkStub *ts) {
	// Send size bytes of serialdata
	uint32_t i;
	SANSGRID_UNION(SansgridSerial, SGSU) sg_serial_union;

	fail_if((ts == NULL), "TalkStub is NULL!");

	fail_if((ts->FPTR_PIPE_WRITE == NULL), "Write descriptor is NULL!");
	fail_if((sg_serial == NULL), "sg_serial is NULL!");

	sg_serial_union.formdata = sg_serial;
	
	sem_post(&ts->writelock);
	sem_wait(&ts->readlock);
	for (i=0; i<size && ts->FPTR_PIPE_WRITE; i++) {
		putc(sg_serial_union.serialdata[i], ts->FPTR_PIPE_WRITE);
	}
	
	return 0;
}



int8_t unixpipeReceive(SansgridSerial **sg_serial, uint32_t *size, TalkStub *ts) {
	// Receive serialdata, size of packet stored in size
	uint32_t i;
	char lptr[sizeof(SansgridSerial)+1];

	if (!ts->valid_read) {
		return 1;
	}
	fail_if((ts == NULL), "TalkStub is NULL!");
	mark_point();
	if (ts->FPTR_PIPE_READ == NULL) {
#if TESTS_DEBUG_LEVEL > 0
		printf("Read file descriptor is null!\n");
#endif
		return -1;
	}
	mark_point();

	// Read from the pipe 
	sem_post(&ts->readlock);
	sem_wait(&ts->writelock);
	
	mark_point();
	for (i=0; i<(sizeof(SansgridSerial)) && ts->FPTR_PIPE_READ; i++) {
		lptr[i] = fgetc(ts->FPTR_PIPE_READ);
	}

	mark_point();

	if (i < sizeof(SansgridSerial)) {
#if TESTS_DEBUG_LEVEL > 0
		printf("Dropping Packet at %i of %i\n", i, 
				sizeof(SansgridSerial));
#endif
		return 1;
	}

	mark_point();
	*sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memcpy(*sg_serial, lptr, sizeof(SansgridSerial));
	*size = sizeof(SansgridSerial);

	mark_point();
	return 0;
}



// vim: ft=c ts=4 noet sw=4:

