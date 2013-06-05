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
/// \file

#include <sys/stat.h>		// umask()
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), setsid(), chdir(), close(), sleep()
#include <syslog.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <sgSerial.h>
#include "sansgrid_daemon.h"
#include "../dispatch/dispatch.h"
#include "../routing_table/routing_table.h"
#include "../sansgrid_router.h"
#include "../communication/sg_tcp.h"
#include "../payload_handlers/payload_handlers.h"
#include "../routing_table/auth_status.h"



/** 
 * \brief Check to see if the daemon is running
 *
 * \returns
 * if the daemon is running, return the PID. \n
 * Otherwise return 0.
 */
int isRunning(void) {
	// Check to see if the sansgrid daemon is running
	pid_t sgpid;
	FILE *FPTR = NULL;
	char config_path[150];
	getSansgridControlDir(config_path);
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



/// Initialize and fork off the daemon
int daemon_init(void) {
	// Initialize and fork off the daemon

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



/**
 * \brief Insert a packet into the Sansgrid System
 *
 * \param[in]	s2	Opened Socket
 * \param[in]	str	Packet in a null-terminated string
 */
int sgConfigInsertPacket(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// Interpret an intrarouter packet and enqueue it for processing
	char *packet;
	SansgridSerial *sg_serial = NULL;
	int exit_code = 0;
	if ((packet = strstr(str, DELIM_KEY)) != NULL) {
		// Got a packet from the server
		syslog(LOG_DEBUG, "sansgrid daemon: interpreting packet: %s", packet);
		sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
		exit_code = sgServerToRouterConvert(strstr(packet, DELIM_KEY),
				sg_serial);
		if (exit_code == -1) {
			strcpy(str, "packet conversion failed");
			syslog(LOG_NOTICE, "sansgrid daemon: got bad packet");
		} else {
			strcpy(str, "packet enqueued");
			queueEnqueue(dispatch, sg_serial);
			syslog(LOG_DEBUG, "sansgrid daemon: enqueued a packet from client");
		}
		sg_serial = NULL;
	} else {
		strcpy(str, "No packet found");
		syslog(LOG_NOTICE, "sansgrid daemon: didn't get a packet");
	}
	socketDoSend(s2, str);
	return exit_code;
}



/**
 * \brief Drop a device from the routing table
 *
 * \param[in]	s2	An open socket
 * \param[in]	str	device to drop, or "all" to drop all devices
 */
int sgConfigDropDevice(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// drop a device
	uint32_t device = 0;
	uint8_t ip_addr[IP_SIZE];
	int exit_code = 0;
	syslog(LOG_NOTICE, "Dropping device");
	if (strlen(str) <= strlen("drop")) {
		strcpy(str, "No device specified");
	} else if (!strcmp(str, "drop all")) {
		// drop all devices
		routerFreeAllDevices(routing_table);
		strcpy(str, "All devices freed");
	} else if (!strcmp(str, "drop queue")) {
		// drop queue
		router_opts.dumping_dispatch = 1;
		router_opts.dispatch_pause = 0;
	} else if ((device = atoi(&str[5])) != 0) {
		// drop device
		routingTableRDIDToIP(routing_table, device, ip_addr);
		if (routingTableLookup(routing_table, ip_addr) == 1) {
			if (routerFreeDevice(routing_table, ip_addr) == -1) {
				sprintf(str, "Couldn't free device: %i", device);
			} else {
				sprintf(str, "Device freed: %i", device);
			}
		} else {
			sprintf(str, "Device not found: %i", device);
		}
	} else {
		strcpy(str, "Bad device given");
	}
	socketDoSend(s2, str);
	return exit_code;
}



/**
 * Get the URL of the server
 */
int sgConfigGetURL(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// return the url
	strcpy(str, router_opts.serverip);
	socketDoSend(s2, str);
	return 0;
}


/**
 * Get the key for the router-->server
 */
int sgConfigGetKey(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// return the key
	if (router_opts.serverkey[0] == '\0') {
		// no key
		strcpy(str, "No Key");
	} else {
		strcpy(str, router_opts.serverkey);
	}
	socketDoSend(s2, str);
	return 0;
}



/// Set the URL of the server
int sgConfigSetURL(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// Set a new server URL
	int exit_code = 0;
	if (strlen(str) > 4) {
		memcpy(router_opts.serverip, &str[4], sizeof(router_opts.serverip));
		strcpy(str, "Successfully changed server IP");
	}
	else {
		strcpy(str, "Couldn't change server IP");
		exit_code = -1;
	}
	socketDoSend(s2, str);
	return exit_code;
}



/// Set the Key for the server
int sgConfigSetKey(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// Set a new server key
	int exit_code = 0;
	if (strlen(str) > 4) {
		memcpy(router_opts.serverkey, &str[4], sizeof(router_opts.serverkey));
		strcpy(str, "Successfully set server key");
	} else {
		strcpy(str, "Couldn't set server key");
		exit_code = -1;
	}
	socketDoSend(s2, str);
	return exit_code;
}



/// Set the heartbeat period
int sgConfigSetHeartbeat(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// set heartbeat period
	if (strlen(str) > 10) {
		int hb = atoi(&str[10]);
		if (hb != 0) {
			router_opts.heartbeat_period = hb;
			sem_post(&router_opts.hb_wait);
			strcpy(str, "Changed Heartbeat Period");
		} else {
			strcpy(str, "Couldn't change Heartbeat Period");
		}
	} else {
		strcpy(str, "No interval given");
	}
	socketDoSend(s2, str);
	return 0;
}



/**
 * \brief Set the authentication level
 * Valid levels are loose, filtered, and strict
 */
int sgConfigSetAuth(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// change authentication
	int exit_code = 0;
	if (strlen(str) > 5) {
		if (strstr(str, "strict")) {
			routingTableRequireStrictAuth(routing_table);
			strcpy(str, "Auth is strictly enforced");
		} else if (strstr(str, "loose")) {
			routingTableAllowLooseAuth(routing_table);
			strcpy(str, "Auth is loosely enforced");
		} else if (strstr(str, "filtered")) {
			routingTableSetAuthFiltered(routing_table);
			strcpy(str, "Auth is enforced for packets");
		} else {
			strcpy(str, "Not a valid option");
		}
	} else {
		strcpy(str, "No option given");
		exit_code = -1;
	}
	socketDoSend(s2, str);
	return exit_code;
}



/**
 * Set the network status: shown, or not shown
 */
int sgConfigSetNetwork(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// Set the network status: shown, or not shown
	int exit_code = 0;
	if (strstr(str, "hidden")) {
		router_opts.hidden_network = 1;
		strcpy(str, "Hiding Network");
	} else if (strstr(str, "shown")) {
		router_opts.hidden_network = 0;
		strcpy(str, "Showing Network");
	} else {
		strcpy(str, "Not a valid option");
		exit_code = -1;
	}
	socketDoSend(s2, str);
	return exit_code;
}



/// Get the status of the router
int sgConfigGetStatus(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// Get the status of the router
	syslog(LOG_DEBUG, "sansgrid daemon: checking status");
	//sprintf(str, "%i", routingTableGetDeviceCount(routing_table));
	int devnum = routingTableGetDeviceCount(routing_table);
	int strictness = routingTableIsAuthStrict(routing_table);
	do {
		sprintf(str, "Routing Table Status:\n");
		if (socketDoSend(s2, str) < 0) break;
		// Print ESSID
		sprintf(str, "\tESSID:\t\t\t%s\n", router_opts.essid);
		if (socketDoSend(s2, str) < 0) break;
		// Print whether the network is hidden or not
		sprintf(str, "\tHidden:\t\t\t");
		if (router_opts.hidden_network == 1) {
			strcat(str, "Yes\n");
		} else {
			strcat(str, "No\n");
		}
		if (socketDoSend(s2, str) < 0) break;
		// print whether or not the authentication is strict
		sprintf(str, "\tAuthentication:\t\t");
		if (strictness == DEV_AUTH_STRICT) {
			strcat(str, "Strict\n");
		} else if (strictness == DEV_AUTH_FILTERED) {
			strcat(str, "Filtered\n");
		} else if (strictness == DEV_AUTH_LOOSE) {
			strcat(str, "Loose\n");
		} else {
			strcat(str, "Unknown\n");
		}
		if (socketDoSend(s2, str) < 0) break;
		// Print how often we heartbeat a device
		sprintf(str, "\tHeartbeat Period:\t%i seconds\n",
			   router_opts.heartbeat_period);
		if (socketDoSend(s2, str) < 0) break;
		// Print whether we're pulling from the dispatch
		// 		or whether we're holding everything on the dispatch
		sprintf(str, "\nDispatch Status:\n");
		if (socketDoSend(s2, str) < 0) break;
		if (router_opts.dispatch_pause)
			sprintf(str, "\tDispatch Paused\n");
		else
			sprintf(str, "\tDispatch Running\n");
		if (socketDoSend(s2, str) < 0) break;
		// Print how full the dispatch is
		sprintf(str, "\tQueued: %i of %i\n", 
				queueSize(dispatch), queueMaxSize(dispatch));
		if (socketDoSend(s2, str) < 0) break;
		// Print the routing table
		sprintf(str, "\nDevices:\n");
		if (socketDoSend(s2, str) < 0) break;
		sprintf(str, " \
rdid                IP address                 status  Last Packet\n");
		if (socketDoSend(s2, str) < 0) break;
		for (int i=0; i<devnum; i++) {
			routingTableGetStatus(routing_table, i, str);
			syslog(LOG_DEBUG, "sansgrid daemon: sending back: %s", str);
			if (socketDoSend(s2, str) < 0) {
				break;
			}
		}
		return 0;
	} while (0);
	return -1;
}



/// Get the number of devices in the network
int sgConfigGetDevices(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// Get the number of devices
	syslog(LOG_DEBUG, "sansgrid daemon: return # of devices");
	sprintf(str, "%i", routingTableGetDeviceCount(routing_table));
	socketDoSend(s2, str);
	return 0;
}



/// Return true if the network is hidden
int sgConfigCheckNetworkHidden(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// return if the network is hidden (not broadcasting)
	if (router_opts.hidden_network) {
		strcpy(str, "Hidden Network");
	} else {
		strcpy(str, "Shown Network");
	}
	socketDoSend(s2, str);
	return 0;
}



/// Pause the dispatch system
int sgConfigPauseDispatch(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// pause packet sending
	if (!router_opts.dispatch_pause) {
		strcpy(str, "Pausing payload handling");
		socketDoSend(s2, str);
	}
	router_opts.dispatch_pause = 1;
	return 0;
}



/// Resume the dispatch system
int sgConfigResumeDispatch(int s2, char str[SG_SOCKET_BUFF_SIZE]) {
	// resume packet sending
	if (router_opts.dispatch_pause) {
		strcpy(str, "Resuming payload handling");
		socketDoSend(s2, str);
	}
	router_opts.dispatch_pause = 0;
	return 0;
}



/// Listen on the socket
int sgSocketListen(void) {
	// Wait for a command from a client
	uint32_t i;
	int s, s2;								// socket info
	struct sockaddr_un local, remote;		// socket addresses
	socklen_t len;							// socket lengths
	char str[SG_SOCKET_BUFF_SIZE];			// socket transmissions
	char socket_path[150];					// socket locations
	int shutdown_server = 0;				// break-out for main loop

	struct Command {
		// Key-value pair for matching a command to a function
		char *token;
		// the integer is an open socket,
		// the char* is data to pass into the function
		int(*callback)(int, char*);
	};

	getSansgridControlDir(socket_path);

	// Create a socket endpoint
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Create the socket path
	// FIXME: Check for permissions errors here
	local.sun_family = AF_UNIX;
	mkdir(socket_path, 0755);
	strcat(socket_path, "/command_socket");
	strcpy(local.sun_path, socket_path);

	unlink(local.sun_path);

	// bind the name to the socket
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	// Make sure everyone can write to the socket
	chmod(socket_path, 0777);

	// listen for socket connections
	if (listen(s, 5) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	do {
		// Normal commands that can be called 
		struct Command command[] = {
			{ "packet=",		sgConfigInsertPacket },
			{ "packet:", 		sgConfigInsertPacket },
			{ "drop ", 			sgConfigDropDevice },
			{ "url=", 			sgConfigSetURL },
			{ "url", 			sgConfigGetURL },
			{ "key=", 			sgConfigSetKey },
			{ "key", 			sgConfigGetKey },
			{ "heartbeat=",		sgConfigSetHeartbeat },
			{ "auth=",			sgConfigSetAuth },
			{ "network=",		sgConfigSetNetwork },
			{ "status",			sgConfigGetStatus },
			{ "devices",		sgConfigGetDevices },
			{ "is-hidden",		sgConfigCheckNetworkHidden },
			{ "pause",			sgConfigPauseDispatch },
			{ "resume",			sgConfigResumeDispatch },
		};

		// Block until a connection appears
		// Then Accept the connection
		if ((s2 = accept(s, (struct sockaddr*)&remote, &len)) == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// Receive and interpret the data
		if (socketDoReceive(s2, str) == -1) {
			syslog(LOG_WARNING, "sansgrid daemon: receive error");
			break;
		}
		syslog(LOG_DEBUG, "sansgrid daemon: received data: %s", str);


		// Interpret command
		if (!strcmp(str, "kill")) {
			// Kill the server
			shutdown_server = 1;
			syslog(LOG_NOTICE, "sansgrid daemon: shutting down");
			strcpy(str, "Shutting down daemon...");
			routerFreeAllDevices(routing_table);
			socketDoSend(s2, str);
		} else {
			for (i=0; i<sizeof(command)/sizeof(command[0]); i++) {
				if (!strncmp(command[i].token, str, strlen(command[i].token))) {
					command[i].callback(s2, str);
					break;
				}
			}
		}
		syslog(LOG_INFO, "sansgrid daemon: sending back: %s", str);

		close(s2);
	} while (!shutdown_server);

	// cleanup

	return 0;
}



// vim: ft=c ts=4 noet sw=4:
