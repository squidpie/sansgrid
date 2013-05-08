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
#include <syslog.h>
#include <errno.h>


#define SG_SOCKET_BUFF_SIZE 1000

void usage(int status);



void *dispatchRuntime(void *arg) {
	SansgridSerial *sg_serial;

	while (1) {
		if (queueDequeue(dispatch, (void**)&sg_serial) == -1) {
			syslog(LOG_ERR, "Dispatch Queue Failed, Quitting");
			exit(EXIT_FAILURE);
		}
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

	memset(sg_hb.padding, 0x0, sizeof(sg_hb.padding));
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
		if (routingTableFindNextDevice(routing_table, ip_addr) != 0) {
			syslog(LOG_DEBUG, "heartbeat: sending to device %u", ip_addr[IP_SIZE-1]);
			memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
			sgSerialSend(&sg_serial, sizeof(SansgridSerial));
		}
	}

	pthread_exit(arg);
}

void *flyRuntime(void *arg) {
	// handle broadcast of ESSID
	SansgridFly sg_fly;
	SansgridSerial sg_serial;
	memset(&sg_fly, 0x0, sizeof(SansgridFly));
	memset(&sg_serial, 0x0, sizeof(SansgridSerial));
	sg_fly.datatype = SG_FLY;
	while (1) {
		routingTableGetEssid(routing_table, sg_fly.network_name);
		memcpy(sg_serial.payload, &sg_fly, sizeof(SansgridFly));
		sg_serial.control = SG_SERIAL_CTRL_VALID_DATA;
		routingTableGetBroadcast(routing_table, sg_serial.ip_addr);
		if (!router_opts.hidden_network)
			routerHandleFly(routing_table, &sg_serial);
		sleep(1);
	}

	pthread_exit(arg);
}
	

void fnExit(void) {
	printf("Exiting\n");
}


int socketDoSend(int s, const char *str) {
	if (send(s, str, strlen(str), 0) == -1) {
		return -1;
	} else {
		return 0;
	}
}

int socketDoReceive(int s, char *str) {
	int t;
	if ((t = recv(s, str, SG_SOCKET_BUFF_SIZE, 0)) > 0) {
		// strip newline
		if (str[t-1] == '\n') {
			str[t-1] = '\0';
		} else {
			str[t] = '\0';
		}
		return t;
	} else {
		// problems
		if (t < 0) perror ("recv");
		return -1;
	}
}



int sgSocketListen(void) {
	// Wait for a command from a client
	int s, s2;								// socket info
	struct sockaddr_un local, remote;		// socket addresses
	socklen_t len;							// socket lengths
	char str[SG_SOCKET_BUFF_SIZE];			// socket transmissions
	char home_path[150];
	char socket_path[150];					// socket locations
	SansgridSerial sg_serial;
	int exit_code;
	char *packet;

	getSansgridDir(home_path);
	strcpy(socket_path, home_path);

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
	chmod(socket_path, 0777);

	// listen for socket connections
	if (listen(s, 5) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	int shutdown_server = 0;

	do {
		// Block until a connection appears
		// Then Accept the connection
		if ((s2 = accept(s, (struct sockaddr*)&remote, &len)) == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// Receive and interpret the data
		if (socketDoReceive(s2, str) == -1) {
			syslog(LOG_ERR, "sansgrid daemon: receive error");
			break;
		}
		syslog(LOG_DEBUG, "sansgrid daemon: received data: %s", str);


		// Interpret command
		if (!strcmp(str, "kill")) {
			// Kill the server
			shutdown_server = 1;
			syslog(LOG_DEBUG, "sansgrid daemon: shutting down");
			socketDoSend(s2, str);
		} else if ((packet = strstr(str, DELIM_KEY)) != NULL) {
			// Got a packet from the server
			syslog(LOG_DEBUG, "sansgrid daemon: interpreting packet: %s", packet);
			exit_code = sgServerToRouterConvert(strstr(packet, DELIM_KEY),
					&sg_serial);
			if (exit_code == -1) {
				strcpy(str, "bad packet");
				syslog(LOG_DEBUG, "sansgrid daemon: got bad packet");
			} else {
				strcpy(str, "packet accepted");
				queueEnqueue(dispatch, &sg_serial);
				syslog(LOG_DEBUG, "sansgrid daemon: got good packet");
				if (socketDoSend(s2, str) == -1) {
					close(s2);
					continue;
				}
			}
		} else if (strstr(str, "drop") != NULL) {
			// drop a device
			uint32_t device = 0;
			uint8_t ip_addr[IP_SIZE];
			syslog(LOG_DEBUG, "Dropping device");
			if (strlen(str) <= strlen("drop")) {
				strcpy(str, "No device specified");
			} else if ((device = atoi(&str[5])) != 0) {
				// drop device
				routingTableRDIDToIP(routing_table, device, ip_addr);
				if (routingTableLookup(routing_table, ip_addr) == 1) {
					routerFreeDevice(routing_table, ip_addr);
					strcpy(str, "Device freed");
				} else {
					strcpy(str, "Device not found");
				}
			} else {
				strcpy(str, "Bad device given");
			}
			if (socketDoSend(s2, str) < 0) {
				close(s2);
				continue;
			}
		} else if (!strcmp(str, "status")) {	
			syslog(LOG_DEBUG, "sansgrid daemon: checking status");
			//sprintf(str, "%i", routingTableGetDeviceCount(routing_table));
			int devnum = routingTableGetDeviceCount(routing_table);
			for (int i=0; i<devnum; i++) {
				routingTableGetStatus(routing_table, i, str);
				syslog(LOG_DEBUG, "sansgrid daemon: sending back: %s", str);
				if (socketDoSend(s2, str) < 0) {
					break;
				}
			}
		} else if (!strcmp(str, "devices")) {
			// Get the number of devices
			syslog(LOG_DEBUG, "sansgrid daemon: return # of devices");
			sprintf(str, "%i", routingTableGetDeviceCount(routing_table));
			socketDoSend(s2, str);
		} else if (!strcmp(str, "hide-network")) {
			// Don't broadcast essid
			syslog(LOG_INFO, "Sansgrid Daemon: Hiding ESSID network");
			router_opts.hidden_network = 1;
			strcpy(str, "Hiding Network");
		} else if (!strcmp(str, "show-network")) {
			// Broadcast essid
			syslog(LOG_INFO, "Sansgrid Daemon: Showing ESSID network");
			router_opts.hidden_network = 0;
			strcpy(str, "Showing Network");
		}
		syslog(LOG_DEBUG, "sansgrid daemon: sending back: %s", str);

		close(s2);
	} while (!shutdown_server);

	// cleanup

	return 0;
}





int sgSocketSend(const char *data, const int size) {
	int s, t;
	socklen_t len;
	struct sockaddr_un remote;
	char str[SG_SOCKET_BUFF_SIZE];
	char socket_path[150];
	char home_path[150];

	getSansgridDir(home_path);
	strcpy(socket_path, home_path);

	// FIXME:
	// 	I'll probably have to add a function
	// 	to find the actual datadir; check the current directory,
	// 	then check the PREFIX/share dir?
	// 	But then it would break if there was a file with the same name in the current dir
	// 	but wasn't for the same purpose. And it still might cause stale scripts...
	//printf("%s\n", DATADIR);
	// Make sure the server is running 
	// before we try to send a command
	if (!isRunning()) {
		printf("sansgridrouter isn't running\n");
		exit(EXIT_SUCCESS);
	}
	if (size > SG_SOCKET_BUFF_SIZE) {
		printf("String is too large!");
		return -1;
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
		syslog(LOG_ERR, "sansgrid client: connect error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Send the command
	syslog(LOG_DEBUG, "sansgrid client: sending data");
	if (socketDoSend(s, data) == -1) {
		syslog(LOG_ERR, "sansgrid client: send error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Get the ACK back from the server
	while ((t = socketDoReceive(s, str)) > 0) {
		// check to see if the server got the kill message
		// Tell the user that the daemon is shutting down
		if (!strcmp(str, "kill")) {
			syslog(LOG_INFO, "sansgrid client: Shutting down daemon...\n");
		} else {
			printf("%s\n", str);
		}
	}
	// cleanup
	close(s);

	return 0;
}


int sgStorePID(pid_t pid) {
	FILE *PIDFILE;
	char config_path[150];
	char pidpath[150];
	char home_path[150];
	getSansgridDir(home_path);
	strcpy(config_path, home_path);
	snprintf(pidpath, 150, "%s/sansgridrouter.pid", config_path);
	if ((PIDFILE = fopen(pidpath, "w")) == NULL) {
		perror("fopen");
		return -1;
	}
	printf("Running as process %i\n", pid);
	fprintf(PIDFILE, "%i\n", pid);
	fclose(PIDFILE);

	return 0;
}

int parseConfFile(const char *path, RouterOpts *ropts) {
	// parse a config file
}	


int main(int argc, char *argv[]) {
	pthread_t 	serial_read_thread,		// thread for reading over SPI
				dispatch_thread,		// thread for reading from dispatch
				server_read_thread,		// thread for reading from server
				heartbeat_thread,		// thread for pinging sensors
				fly_thread;				// thread for broadcasting ESSID

	int c;								// getopt var
	char *option = NULL;				// getopt var
	int32_t no_daemonize = 0;			// bool: should we run in foreground?
	char home_path[150];
	char config_path[150];				// Sansgrid Dir
	pid_t sgpid;						// Sansgrid PID
	char payload[400];
	uint8_t ip_addr[IP_SIZE];
	SansgridHatching sg_hatch;
	SansgridSerial sg_serial;


	memset(&router_opts, 0x0, sizeof(RouterOpts));

	getSansgridDir(home_path);
	strcpy(config_path, home_path);

	// Parse arguments with getopt
	while (1) {
		const struct option long_options[] = {
			{"foreground",	no_argument, 		&no_daemonize, 	1},
			{"daemon", 		no_argument, 		&no_daemonize, 	0},
			{"packet",		required_argument, 	0,				'p'},
			{"help", 		no_argument, 		0, 				'h'},
			{"version", 	no_argument, 		0, 				'v'},
			{"serverip",	required_argument,	0,				's'},
			{"serverkey",	required_argument,	0,				'k'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "d:fhp:vs:k:", long_options, &option_index);
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
			case 'k':
				// Server key is given
				break;
			case 'p':
				// Payload is given
				sprintf(payload, "packet: %s", optarg);
				sgSocketSend(payload, strlen(payload));
				exit(EXIT_SUCCESS);
				break;
			case 's':
				// Server IP address given
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
			sgSocketSend("status", 7);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "devices")) {
			// get the number of devices
			sgSocketSend("devices", 8);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "hide-network")) {
			// hide the network
			sgSocketSend("hide-network", 13);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "show-network")) {
			// show the network
			sgSocketSend("show-network", 13);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "drop")) {
			// drop a device
			if (optind < argc) {
				char doDrop[1000];
				sprintf(doDrop, "drop %s", argv[optind]);
				sgSocketSend(doDrop, strlen(doDrop));
				exit(EXIT_SUCCESS);
			}
		} else if (!strcmp(option, "running")) {
			// check to see if the daemon is running
			if ((sgpid = isRunning()) != 0) {
				printf("Running as process %i\n", sgpid);
			} else {
				printf("sansgridrouter is not running\n");
			}
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "change-server-ip")) {
			// Change the server IP address for the running daemon
			printf("Not implemented yet\n");
			exit(EXIT_FAILURE);
		} else if (!strcmp(option, "change-server-key")) {
			// Change the server key for the running daemon
			printf("Not implemented yet\n");
			exit(EXIT_FAILURE);
		} else {
			// bad option
			printf("Unknown Arg: %s\n", option);
			exit(EXIT_FAILURE);
		}
	}

	if (isRunning()) {
		printf("sansgridrouter already running\n");
		return EXIT_FAILURE;
	}

	

	// Should we run in the foreground, or the background?
	if (!no_daemonize) {
		// Run in the background
		int excode = daemon_init();
		if (excode == EXIT_FAILURE)
			exit(EXIT_FAILURE);
	} else {
		sgStorePID(getpid());
	}

	atexit(fnExit);

	// Initialize routing subsystem
	dispatch = queueInit(200);
	// TODO: Assign ESSID from config file
	routing_table = routingTableInit(router_base, "Stock ESSID");
	void *arg;

	// TODO: set IP address correctly
	memset(&sg_hatch, 0x0, sizeof(SansgridHatching));
	memset(&sg_serial, 0x0, sizeof(SansgridSerial));
	memset(ip_addr, 0x0, IP_SIZE);
	ip_addr[IP_SIZE-1] = 0x1;
	sg_hatch.datatype = SG_HATCH;
	memcpy(sg_hatch.ip, ip_addr, IP_SIZE);
	memcpy(&sg_serial.payload, &sg_hatch, sizeof(SansgridHatching));
	sg_serial.control = SG_SERIAL_CTRL_VALID_DATA;
	memcpy(sg_serial.ip_addr, ip_addr, IP_SIZE);
	routerHandleHatching(routing_table, &sg_serial);


	// Spin off readers/writers
	pthread_create(&serial_read_thread, NULL, spiReaderRuntime, dispatch);
	pthread_create(&server_read_thread, NULL, serverReaderRuntime, dispatch);
	pthread_create(&dispatch_thread, NULL, dispatchRuntime, dispatch);
	pthread_create(&heartbeat_thread, NULL, heartbeatRuntime, dispatch);
	pthread_create(&fly_thread, NULL, flyRuntime, dispatch);

	// Listen for commands or data from the server
	sgSocketListen();

	// Finished. Shut system down
	pthread_cancel(serial_read_thread);
	pthread_cancel(server_read_thread);
	pthread_cancel(dispatch_thread);
	pthread_cancel(heartbeat_thread);
	pthread_cancel(fly_thread);

	pthread_join(serial_read_thread, &arg);
	pthread_join(server_read_thread, &arg);
	pthread_join(dispatch_thread, &arg);
	pthread_join(heartbeat_thread, &arg);
	pthread_join(fly_thread, &arg);

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
  -p  --packet [PACKET]      Send a sansgrid payload to the server\n\
  -h, --help                 display this help and exit\n\
  -v, --version              output version information and exit\n\
\n\
      status                 show status of devices\n\
      kill                   shutdown the router daemon\n\
      running                check to see if router daemon is running\n\
      devices                print number of devices tracked\n\
      hide-network           don't broadcast essid\n\
      show-network           broadcast essid\n\
	  drop [DEVICE]          drop a device");
	}
	exit(status);
}

// vim: ft=c ts=4 noet sw=4:
