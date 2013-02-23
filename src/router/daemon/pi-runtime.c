/* Daemon initialization
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

#include <sys/stat.h>		// umask()
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), setsid(), chdir(), close(), sleep()
#include <syslog.h>
//#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>


#if defined(SYSLOG) && defined(LOG_FILE)
#undef LOG_FILE
#endif

#if defined(LOG_DAEMON) && !defined(LOG_CRON)
#define LOG_CRON LOG_DAEMON
#endif

#ifndef FACILITY
#define FACILITY LOG_CRON
#endif

#if defined(SYSLOG)
static int syslog_open = FALSE;
#endif
	

int main(void) {
	pid_t pid;
	pid_t sid;

	//int fd;

	pid = fork();

	// exit if fork failed
	if (pid < 0)
		exit(EXIT_FAILURE);

	// kill parent process
	if (pid > 0)
		exit(EXIT_SUCCESS);

	// change the file mode mask
	umask(0);

	// open logs here
	
	// create new sid for child process
	sid = setsid();
	if (sid < 0) {
		// log any failure
		exit(EXIT_FAILURE);
	}

	// move working directory to root
	if ((chdir("/")) < 0) {
		// log any failure here
		exit(EXIT_FAILURE);
	}

	// close standard file descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// daemon-specific initialization goes here
	


	// The Big Loop
	while (1) {
		// do stuff here
		sleep(30);	// wait 30 seconds
	}

	exit(EXIT_SUCCESS);
}




// vim: ft=c ts=4 noet sw=4:
