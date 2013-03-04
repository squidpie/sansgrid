/*
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
#include <stdlib.h>
#include <unistd.h>

#ifndef __H_ROUTING__
#define __H_ROUTING__
#include "../../routing/routing.h"
#endif

#include "radio-stub.h"

void radioStubRuntime(int fildes[2]) {
	// Stub for the radio module
	// Write to a named pipe

	int i;
	char lptr[5] = "1";
	FILE *F_HANDLE;
	char filedata[101];

	if (!(F_HANDLE = fopen("test/rstubin.fifo", "w"))) {
		printf("ERROR: Can't open rstubin.fifo\n");
		exit(EXIT_FAILURE);
	}
	close(fildes[0]);

	for (i=0; i<10; i++) {
		// write ping 10 times, then signal exiting using 
		// the unnamed pipe
		snprintf(filedata, 100, "Ping");
		fwrite(filedata, sizeof(char), 100, F_HANDLE);
	}


	write(fildes[1], lptr, 5);
	close(fildes[1]);

	return;
}


// vim: ft=c ts=4 noet sw=4:
