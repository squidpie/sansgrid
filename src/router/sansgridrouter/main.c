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
#include "../../payloads.h"
#include "payload_handlers/payload_handlers.h"
#include "../../sg_serial.h"
#include <sys/stat.h>
#include <sys/un.h>


void usage(int status);



void *dispatchRuntime(void *arg) {
	SansgridSerial *sg_serial;

	while (1) {
		if (queueDequeue(dispatch, (void**)&sg_serial) == -1) 
			exit(EXIT_FAILURE);
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
	for (int i=0; i<IP_SIZE; i++)
		sg_serial.origin_ip[i] = 0x0;
	sg_serial.origin_ip[IP_SIZE-1] = 0x1;
	sg_serial.origin = 0x0;
	sg_serial.timestamp = 0x0;
	while (1) {
		count = routingTableGetDeviceCount(routing_table);
		sleepMicro(HEARTBEAT_UINTERVAL / count);
		routingTableFindNextDevice(routing_table, ip_addr);
		memcpy(&sg_serial.dest_ip, ip_addr, IP_SIZE);
		sgSerialSend(&sg_serial, sizeof(SansgridSerial));
	}

	pthread_exit(arg);
}
	
	

void fnExit(void) {
	printf("Exiting\n");
}



int sgSocketListen(void) {
	int s, s2;
	struct sockaddr_un local, remote;
	socklen_t len;
	char str[100];
	char socket_path[150];
	getSansgridDir(socket_path);

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	local.sun_family = AF_UNIX;
	mkdir(socket_path, 0755);
	strcat(socket_path, "/command_socket");
	strcpy(local.sun_path, socket_path);

	unlink(local.sun_path);

	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(s, 5) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	int shutdown_server = 0;
	do {
		int done, n;
		//printf("Waiting for a connection...\n");
		if ((s2 = accept(s, (struct sockaddr*)&remote, &len)) == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		
		//printf("Connected.\n");
		done = 0;
		do {
			n = recv(s2, str, 100, 0);
			if (n <= 0) {
				if (n < 0) perror("recv");
				done = 1;
			}
			if (!done) {
				if (str[n-1] == '\n')
					str[n-1] = '\0';
				else
					str[n] = '\0';
				//printf("Received %s\n", str);
				if (!strcmp(str, "kill")) {
					shutdown_server = 1;
					done = 1;
				}

				if (send(s2, str, n, 0) < 0) {
					perror("send");
					done = 1;
				}
			}
		} while (!done);
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

	if (!isRunning()) {
		printf("sansgridrouter isn't running\n");
		exit(EXIT_SUCCESS);
	}
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	//printf("Trying to connect...\n");

	remote.sun_family = AF_UNIX;

	strcat(socket_path, "/command_socket");
	strcpy(remote.sun_path, socket_path);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr*)&remote, len) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	//printf("Connected.\n");

	//while (printf("> "), fgets(str, 100, stdin), !feof(stdin)) {
	if (send(s, data, size, 0) == -1) {
		perror("send");
		exit(EXIT_FAILURE);
	}

	if ((t = recv(s, str, 100, 0)) > 0) {
		if (str[t-1] == '\n') {
			str[t-1] = '\0';
		} else {
			str[t] = '\0';
		}
		if (!strcmp(str, "kill")) {
			printf("Shutting down daemon...\n");
		}
		//printf("echo> %s\n", str);
		//if (!strcmp(str, "stop")) {
		//	break;
		//}
	} else {
		if (t < 0) perror ("recv");
		else printf("Server closed connection\n");
		exit(EXIT_FAILURE);
	}
	close(s);

	return 0;
}







int main(int argc, char *argv[]) {
	pthread_t 	serial_read_thread,
				dispatch_thread;
	int c;
	int32_t no_daemonize = 0;
	char *option = NULL;
	char config_path[150];
	pid_t sgpid;

	getSansgridDir(config_path);

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

	while (optind < argc) {
		// deal with non-option argv elements
		option = argv[optind++];
		if (!strcmp(option, "kill")) {
			// kill daemon
			sgSocketSend("kill", 4);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "start")) {
			// daemonize
			int excode = daemon_init();
			if (excode == EXIT_FAILURE)
				exit(EXIT_FAILURE);
			no_daemonize = 1;
		} else if (!strcmp(option, "restart")) {
			// kill and start daemon
			// TODO: Implement
			exit(EXIT_FAILURE);
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
	

	if (!no_daemonize) {
		int excode = daemon_init();
		if (excode == EXIT_FAILURE)
			exit(EXIT_FAILURE);
	}

	atexit(fnExit);
	dispatch = queueInit(200);
	routing_table = routingTableInit(router_base);
	void *arg;

	pthread_create(&serial_read_thread, NULL, spiReaderRuntime, dispatch);
	pthread_create(&dispatch_thread, NULL, dispatchRuntime, dispatch);

	// Listen for commands or data from the server
	sgSocketListen();

	pthread_cancel(serial_read_thread);
	pthread_cancel(dispatch_thread);

	pthread_join(serial_read_thread, &arg);
	pthread_join(dispatch_thread, &arg);

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
