/* Router Starting Point
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

#include "routing_table/heartbeat.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include "sansgrid_router.h"
#include "payload_handlers/payload_handlers.h"
#include "communication/sg_tcp.h"
#include <sys/stat.h>
#include <sys/un.h>
#include <arpa/inet.h>


#ifndef DATADIR
#define DATADIR "../../router_to_server"
#endif

void usage(int status);



void *dispatchRuntime(void *arg) {
	SansgridSerial *sg_serial;

	while (1) {
		if (queueDequeue(dispatch, (void**)&sg_serial) == -1) 
			exit(EXIT_FAILURE);
		// FIXME: Use sgPayloadGetType, defined in payload_handlers.c
		switch (sg_serial->payload[0]) {
			case SG_HATCH:
				routerHandleHatching(routing_table, sg_serial);
				break;
			case SG_FLY:
				routerHandleFly(routing_table, sg_serial);
				break;
			case SG_EYEBALL:
				routerHandleEyeball(routing_table, sg_serial);
				break;
			case SG_PECK:
				routerHandlePeck(routing_table, sg_serial);
				break;
			case SG_SING_WITH_KEY:
			case SG_SING_WITHOUT_KEY:
				routerHandleSing(routing_table, sg_serial);
				break;
			case SG_MOCK_WITH_KEY:
			case SG_MOCK_WITHOUT_KEY:
				routerHandleMock(routing_table, sg_serial);
				break;
			case SG_PEACOCK:
				routerHandlePeacock(routing_table, sg_serial);
				break;
			case SG_NEST:
				routerHandleNest(routing_table, sg_serial);
				break;
			case SG_SQUAWK_SERVER_CHALLENGE_SENSOR:
			case SG_SQUAWK_SENSOR_RESPOND_NO_REQUIRE_CHALLENGE:
			case SG_SQUAWK_SENSOR_RESPOND_REQUIRE_CHALLENGE:
			case SG_SQUAWK_SENSOR_CHALLENGE_SERVER:
			case SG_SQUAWK_SERVER_DENY_SENSOR:
			case SG_SQUAWK_SERVER_RESPOND:
			case SG_SQUAWK_SENSOR_ACCEPT_RESPONSE:
				routerHandleSquawk(routing_table, sg_serial);
				break;
			case SG_HEARTBEAT_ROUTER_TO_SENSOR:
			case SG_HEARTBEAT_SENSOR_TO_ROUTER:
				routerHandleHeartbeat(routing_table, sg_serial);
				break;
			case SG_CHIRP_COMMAND_SERVER_TO_SENSOR:
			case SG_CHIRP_DATA_SENSOR_TO_SERVER:
			case SG_CHIRP_DATA_STREAM_START:
			case SG_CHIRP_DATA_STREAM_CONTINUE:
			case SG_CHIRP_DATA_STREAM_END:
			case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			case SG_CHIRP_SENSOR_DISCONNECT:
				routerHandleChirp(routing_table, sg_serial);
				break;
			default:
				break;
		}
	}
	pthread_exit(arg);
}

void *spiReaderRuntime(void *arg) {
	// Read from SPI and queue data onto dispatch
	uint32_t size;
	SansgridSerial *sg_serial;
	while (1) {
		while (sgSerialReceive(&sg_serial, &size) == -1) {
			sched_yield();
		}
		queueEnqueue(dispatch, sg_serial);
	}
	pthread_exit(arg);
}

void *serverReaderRuntime(void *arg) {
	// Read from the Server and queue data onto dispatch
	uint32_t size;
	SansgridSerial *sg_serial;
	while (1) {
		while (sgTCPReceive(&sg_serial, &size) == -1) {
			sched_yield();
		}
		queueEnqueue(dispatch, sg_serial);
	}
	pthread_exit(arg);
}


void *heartbeatRuntime(void *arg) {
	// handle pings

	int32_t count;
	uint8_t ip_addr[IP_SIZE];
	SansgridSerial sg_serial;
	SansgridHeartbeat sg_hb;
	sg_hb.datatype = SG_HEARTBEAT_ROUTER_TO_SENSOR;

	for (int i=0; i<80; i++)
		sg_hb.padding[i] = 0x0;
	memcpy(&sg_serial.payload, &sg_hb, sizeof(SG_HEARTBEAT_ROUTER_TO_SENSOR));
	while (1) {
		count = routingTableGetDeviceCount(routing_table);
		if (count == 0) {
			// check for possible floating point exceptions
			// if count is 0, you'll get a divide-by-zero error below
			count = 1;
		}
		// FIXME: check to see if interval is below 1 second
		sleep(HEARTBEAT_INTERVAL/count);
		//sleepMicro(HEARTBEAT_UINTERVAL / count);
		routingTableFindNextDevice(routing_table, ip_addr);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		sgSerialSend(&sg_serial, sizeof(SansgridSerial));
	}

	pthread_exit(arg);
}
	
	

void fnExit(void) {
	printf("Exiting\n");
}



int sgSocketListen(void) {
	// Wait for a command from a client
	int s, s2;								// socket info
	struct sockaddr_un local, remote;		// socket addresses
	socklen_t len;							// socket lengths
	char str[100];							// socket transmissions
	char socket_path[150];					// socket locations

	getSansgridDir(socket_path);

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

	// listen for socket connections
	if (listen(s, 5) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	int shutdown_server = 0;

	do {
		int done, n;
		// Block until a connection appears
		// Then Accept the connection
		if ((s2 = accept(s, (struct sockaddr*)&remote, &len)) == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		
		// Receive and interpret the data
		done = 0;
		do {
			n = recv(s2, str, 100, 0);
			// make sure we got something
			if (n <= 0) {
				if (n < 0) perror("recv");
				done = 1;
			}
			if (!done) {
				// Strip newlines
				if (str[n-1] == '\n')
					str[n-1] = '\0';
				else
					str[n] = '\0';

				// Interpret command
				if (!strcmp(str, "kill")) {
					// Kill the server
					shutdown_server = 1;
					done = 1;
				}

				// Send commnad back to client as ACK
				if (send(s2, str, n, 0) < 0) {
					perror("send");
					done = 1;
				}
			}
		} while (!done);

		// cleanup
		close(s2);
	} while (!shutdown_server);

	return 0;
}


int sgSocketSend(const char *data, const int size) {
	int s, t;
	socklen_t len;
	struct sockaddr_un remote;
	char str[100];
	char socket_path[150];
	getSansgridDir(socket_path);

	// FIXME:
	// 	I'll probably have to add a function
	// 	to find the actual datadir; check the current directory,
	// 	then check the PREFIX/share dir?
	// 	But then it would break if there was a file with the same name in the current dir
	// 	but wasn't for the same purpose. And it still might cause stale scripts...
	printf("%s\n", DATADIR);
	// Make sure the server is running 
	// before we try to send a command
	if (!isRunning()) {
		printf("sansgridrouter isn't running\n");
		exit(EXIT_SUCCESS);
	}
	// Create a socket
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Get the Socket Path
	remote.sun_family = AF_UNIX;

	strcat(socket_path, "/command_socket");
	strcpy(remote.sun_path, socket_path);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr*)&remote, len) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	// Send the command
	if (send(s, data, size, 0) == -1) {
		perror("send");
		exit(EXIT_FAILURE);
	}

	// Get the ACK back from the server
	if ((t = recv(s, str, 100, 0)) > 0) {
		// strip newline
		if (str[t-1] == '\n') {
			str[t-1] = '\0';
		} else {
			str[t] = '\0';
		}
		// check to see if the server got the kill message
		// Tell the user that the daemon is shutting down
		if (!strcmp(str, "kill")) {
			printf("Shutting down daemon...\n");
		}
	} else {
		// problems
		if (t < 0) perror ("recv");
		else printf("Server closed connection\n");
		exit(EXIT_FAILURE);
	}
	// cleanup
	close(s);

	return 0;
}



int main(int argc, char *argv[]) {
	pthread_t 	serial_read_thread,		// thread for reading over SPI
				dispatch_thread,		// thread for reading from dispatch
				server_read_thread,		// thread for reading from server
				heartbeat_thread;		// thread for pinging sensors

	int c;								// getopt var
	char *option = NULL;				// getopt var
	int32_t no_daemonize = 0;			// bool: should we run in foreground?
	char config_path[150];				// Sansgrid Dir
	pid_t sgpid;						// Sansgrid PID

	getSansgridDir(config_path);

	// Parse arguments with getopt
	while (1) {
		const struct option long_options[] = {
			{"foreground",	no_argument, 		&no_daemonize, 	1},
			{"daemon", 		no_argument, 		&no_daemonize, 	0},
			{"payload",		required_argument, 	0,				'p'},
			{"help", 		no_argument, 		0, 				'h'},
			{"version", 	no_argument, 		0, 				'v'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "fhpv", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				printf("option %s ", long_options[option_index].name);
				if (optarg)
					printf("With arg %s", optarg);
				printf("\n");
				break;
			case 'f':
				// Run in the foreground
				no_daemonize = 1;
				break;
			case 'h':
				// help
				usage(EXIT_SUCCESS);
				break;
			case 'p':
				// Payload is given
				//memcpy(payload, optarg, strlen(optarg)+1);
				// TODO: Process and send this to the daemon
				break;
			case 'v':
				// version
				// TODO: Give version
				printf("Not implemented yet!\n");
				exit(EXIT_SUCCESS);
				break;
			case '?':
				// getopt_long alreaady printed an error message
				exit(EXIT_FAILURE);
				break;
			default:
				abort();
		}
	}

	// Parse remaining commands
	while (optind < argc) {
		// deal with non-option argv elements
		option = argv[optind++];
		if (!strcmp(option, "kill")) {
			// kill daemon
			sgSocketSend("kill", 4);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "start")) {
			// daemonize
			no_daemonize = 0;
		} else if (!strcmp(option, "restart")) {
			// kill and start daemon
			sgSocketSend("kill", 4);
			sleep(1);
			no_daemonize = 0;
		} else if (!strcmp(option, "status")) {
			// print the status of the router daemon
			// TODO: Implement
			exit(EXIT_FAILURE);
		} else if (!strcmp(option, "running")) {
			// check to see if the daemon is running
			if ((sgpid = isRunning()) != 0) {
				printf("Running as process %i\n", sgpid);
			} else {
				printf("sansgridrouter is not running\n");
			}
			exit(EXIT_SUCCESS);
		} else {
			// bad option
			printf("Unknown Arg: %s\n", option);
			exit(EXIT_FAILURE);
		}
	}
	

	// Should we run in the foreground, or the background?
	if (!no_daemonize) {
		// Run in the background
		int excode = daemon_init();
		if (excode == EXIT_FAILURE)
			exit(EXIT_FAILURE);
	}

	atexit(fnExit);

	// Initialize routing subsystem
	dispatch = queueInit(200);
	routing_table = routingTableInit(router_base);
	void *arg;

	// Spin off readers/writers
	pthread_create(&serial_read_thread, NULL, spiReaderRuntime, dispatch);
	pthread_create(&server_read_thread, NULL, serverReaderRuntime, dispatch);
	pthread_create(&dispatch_thread, NULL, dispatchRuntime, dispatch);
	pthread_create(&heartbeat_thread, NULL, heartbeatRuntime, dispatch);

	// Listen for commands or data from the server
	sgSocketListen();

	// Finished. Shut system down
	pthread_cancel(serial_read_thread);
	pthread_cancel(server_read_thread);
	pthread_cancel(dispatch_thread);
	pthread_cancel(heartbeat_thread);

	pthread_join(serial_read_thread, &arg);
	pthread_join(server_read_thread, &arg);
	pthread_join(dispatch_thread, &arg);
	pthread_join(heartbeat_thread, &arg);

	// Cleanup
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);

	return 0;
}

void usage(int status) {
	if (status != EXIT_SUCCESS)
		printf("Try sansgrid -h\n");
	else {
		printf("Usage: sansgrid [OPTION]\n");
		printf("\
  -f  --foreground           Don't background daemon\n\
  -p  --packet               Send a sansgrid payload to the server\n\
  -h, --help                 display this help and exit\n\
  -v, --version              output version information and exit\n");
	}
	exit(status);
}

// vim: ft=c ts=4 noet sw=4:
