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
#include <pthread.h>
#include <string.h>

#include "sansgrid_daemon.h"
#include "../dispatch/dispatch.h"
#include "../routing_table/routing_table.h"
#include "../sansgrid_router.h"



void getSansgridDir(char wd[150]) {
	// Get the .sansgrid directory path
	// Return success or failure
	// pass the path back in wd
	char *home_path = getenv("HOME");

	if (!home_path) {
		syslog(LOG_NOTICE, "Didn't get home directory");
		sprintf(wd, "/home/pi/.sansgrid");
	} else {
		snprintf(wd, 120, "%s/.sansgrid", home_path);
	}
	// FIXME: check to see if dir exists
	// 			if not, get config from /etc/sansgrid

}

int isRunning(void) {
	// Check to see if the sansgrid daemon is running
	pid_t sgpid;
	FILE *FPTR = NULL;
	char config_path[150];
	getSansgridDir(config_path);
	strncat(config_path, "/sansgridrouter.pid", 150);

	if (!(FPTR = fopen(config_path, "r"))) {
		// no file to be read. Daemon can't be running
		return 0;
	} else if (!fscanf(FPTR, "%d", &sgpid)) { 
		// no pid to be read. Daemon can't be running
		fclose(FPTR);
		return 0;
	}
	fclose(FPTR);

	sprintf(config_path, "/proc/%d/cmdline", sgpid);
	if (!(FPTR = fopen(config_path, "r"))) {
		// PID doesn't exist. Daemon can't be running.
		return 0;
	} else if (!fscanf(FPTR, "%s", config_path)) {
		// Couldn't read. Daemon isn't running
		fclose(FPTR);
		return 0;
	}
	fclose(FPTR);
	return sgpid;
}


int daemon_init(void) {
	pid_t pid;
	pid_t sid;

	pid = fork();

	// exit if fork failed
	if (pid < 0) {
		syslog(LOG_ERR, "Fork failed");
		exit(EXIT_FAILURE);
	}

	// kill parent process
	if (pid > 0) {
		sgStorePID(pid);
		syslog(LOG_INFO, "Daemon fork successful. Killing parent");
		exit(EXIT_SUCCESS);
	}

	// change the file mode mask
	umask(0);

	// open logs here
	
	// create new sid for child process
	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "Couldn't set sid for daemon");
		exit(EXIT_FAILURE);
	}

	// move working directory to root
	if ((chdir("/")) < 0) {
		syslog(LOG_ERR, "Couldn't move working directory to root");
		exit(EXIT_FAILURE);
	}

	// close standard file descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	syslog(LOG_INFO, "Closed standard unix file descriptors");

	return 0;
}




// vim: ft=c ts=4 noet sw=4:
