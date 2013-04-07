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
	
void getSansgridDir(char wd[150]) {
	// Get the .sansgrid directory path
	// Return success or failure
	// pass the path back in wd
	char *home_path = getenv("HOME");

	if (!home_path) {
		printf("ERROR: Can't find home directory\n");
		exit(EXIT_FAILURE);
	}
	snprintf(wd, 120, "%s/.sansgrid", home_path);

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
	FILE *PIDFILE;
	char config_path[150];
	char pidpath[150];

	if (isRunning()) {
		printf("sansgridrouter already running\n");
		return EXIT_FAILURE;
	}

	pid = fork();

	// exit if fork failed
	if (pid < 0)
		exit(EXIT_FAILURE);

	// kill parent process
	if (pid > 0) {
		getSansgridDir(config_path);
		snprintf(pidpath, 150, "%s/sansgridrouter.pid", config_path);
		if ((PIDFILE = fopen(pidpath, "w")) == NULL) {
			perror("fopen");
			// FIXME: Kill child process too (here)
			exit(EXIT_FAILURE);
		}
		printf("Running as process %i\n", pid);
		fprintf(PIDFILE, "%i\n", pid);
		fclose(PIDFILE);
		exit(EXIT_SUCCESS);
	}

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

	return 0;
}




// vim: ft=c ts=4 noet sw=4:
