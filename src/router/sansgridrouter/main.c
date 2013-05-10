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

#define _POSIX_C_SOURCE 200809L
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

static int dispatch_pause = 0;


void *dispatchRuntime(void *arg) {
	SansgridSerial *sg_serial;

	while (1) {
		if (queueDequeue(dispatch, (void**)&sg_serial) == -1) {
			syslog(LOG_ERR, "Dispatch Queue Failed, Quitting");
			exit(EXIT_FAILURE);
		}
		while (dispatch_pause) {
			sleep(1);
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


void *heartbeatRuntime(void *arg) {
	// handle pings

	int32_t count;
	uint8_t ip_addr[IP_SIZE];
	SansgridSerial sg_serial;
	SansgridHeartbeat sg_hb;
	SansgridChirp sg_chirp;
	sg_hb.datatype = SG_HEARTBEAT_ROUTER_TO_SENSOR;
	struct timespec req, rem;
	int hb_status = 0;

	memset(&sg_chirp, 0x0, sizeof(SansgridChirp));
	memset(sg_hb.padding, 0x0, sizeof(sg_hb.padding));
	memcpy(&sg_serial.payload, &sg_hb, sizeof(SG_HEARTBEAT_ROUTER_TO_SENSOR));
	routingTableForEachDevice(routing_table, ip_addr);
	count = routingTableGetDeviceCount(routing_table);
	while (1) {
		do {
			if (HEARTBEAT_INTERVAL/count == 0) {
				// interval is < 1 second
				// sleep in usecs
				req.tv_nsec = ((HEARTBEAT_INTERVAL*1000L)/count)*1000000L;
				req.tv_sec = 0;
				nanosleep(&req, &rem);
				//sleepMicro(HEARTBEAT_INTERVAL*1000000L / count);
			} else {
				sleep(HEARTBEAT_INTERVAL/count);
			}
		} while ((count = routingTableGetDeviceCount(routing_table)) < 2);

		while (dispatch_pause) {
			sleep(1);
		}
		syslog(LOG_DEBUG, "heartbeat: sending to device %u", ip_addr[IP_SIZE-1]);
		memcpy(&sg_serial.ip_addr, ip_addr, IP_SIZE);
		sgSerialSend(&sg_serial, sizeof(SansgridSerial));
		if ((hb_status = routingTableHeartbeatDevice(routing_table, ip_addr)) != 0) {
			// device status changed... either went stale or was lost
			// TODO: Do something here
			if (routingTableIsDeviceLost(routing_table, ip_addr)) {
				// Device was just lost
				// TODO: inform server that device was lost
				syslog(LOG_NOTICE, "Device %i has just been lost", routingTableIPToRDID(routing_table, ip_addr));
			} else if (routingTableIsDeviceStale(routing_table, ip_addr)) {
				// Device just went stale
				// TODO: inform server that device went stale
				syslog(LOG_NOTICE, "Device %i has just gone stale", routingTableIPToRDID(routing_table, ip_addr));
			}
		}
		if (routingTableStepNextDevice(routing_table, ip_addr) == 0) {
			routingTableForEachDevice(routing_table, ip_addr);
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
		while (dispatch_pause) {
			sleep(1);
		}	
		if (!router_opts.hidden_network)
			routerHandleFly(routing_table, &sg_serial);
		sleep(1);
	}

	pthread_exit(arg);
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
	char socket_path[150];					// socket locations
	SansgridSerial sg_serial;
	int exit_code;
	char *packet;

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
			syslog(LOG_WARNING, "sansgrid daemon: receive error");
			break;
		}
		syslog(LOG_DEBUG, "sansgrid daemon: received data: %s", str);


		// Interpret command
		if (!strcmp(str, "kill")) {
			// Kill the server
			shutdown_server = 1;
			syslog(LOG_NOTICE, "sansgrid daemon: shutting down");
			routerFreeAllDevices(routing_table);
			socketDoSend(s2, str);
		} else if ((packet = strstr(str, DELIM_KEY)) != NULL) {
			// Got a packet from the server
			syslog(LOG_DEBUG, "sansgrid daemon: interpreting packet: %s", packet);
			exit_code = sgServerToRouterConvert(strstr(packet, DELIM_KEY),
					&sg_serial);
			if (exit_code == -1) {
				strcpy(str, "bad packet");
				syslog(LOG_NOTICE, "sansgrid daemon: got bad packet");
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
			syslog(LOG_NOTICE, "Dropping device");
			if (strlen(str) <= strlen("drop")) {
				strcpy(str, "No device specified");
			} else if (!strcmp(str, "drop all")) {
				// drop all devices
				routerFreeAllDevices(routing_table);
				strcpy(str, "All devices freed");
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
			if (socketDoSend(s2, str) < 0) {
				close(s2);
				continue;
			}
		} else if (!strcmp(str, "url")) {
			// return the url
			strcpy(str, router_opts.serverip);
			socketDoSend(s2, str);
		} else if (!strcmp(str, "key")) {
			// return the key
			strcpy(str, router_opts.serverkey);
			socketDoSend(s2, str);
		} else if (strstr(str, "url")) {
			// Set a new server URL
			if (strlen(str) > 4) {
				memcpy(router_opts.serverip, &str[4], sizeof(router_opts.serverip));
				strcpy(str, "Successfully changed server IP");
			}
			else {
				strcpy(str, "Couldn't change server IP");
			}
			socketDoSend(s2, str);
		} else if (strstr(str, "key")) {
			// Set a new server key
			if (strlen(str) > 4) {
				memcpy(router_opts.serverkey, &str[4], sizeof(router_opts.serverkey));
				strcpy(str, "Successfully set server key");
			} else {
				strcpy(str, "Couldn't set server key");
			}
			socketDoSend(s2, str);
		} else if (!strcmp(str, "status")) {	
			syslog(LOG_DEBUG, "sansgrid daemon: checking status");
			//sprintf(str, "%i", routingTableGetDeviceCount(routing_table));
			int devnum = routingTableGetDeviceCount(routing_table);
			do {
				sprintf(str, "Routing Table Status:\n");
				if (socketDoSend(s2, str) < 0) break;
				if (dispatch_pause)
					sprintf(str, "\tDispatch Paused\n");
				else
					sprintf(str, "\tDispatch Running\n");
				if (socketDoSend(s2, str) < 0) break;
				sprintf(str, "Dispatch Size: %i of %i\n", 
						queueSize(dispatch), queueMaxSize(dispatch));
				if (socketDoSend(s2, str) < 0) break;
				sprintf(str, "Devices:\n");
				if (socketDoSend(s2, str) < 0) break;
				sprintf(str, " rdid   IP address            auth\n");
				if (socketDoSend(s2, str) < 0) break;
			} while (0);
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
			syslog(LOG_NOTICE, "Sansgrid Daemon: Hiding ESSID network");
			router_opts.hidden_network = 1;
			strcpy(str, "Hiding Network");
			socketDoSend(s2, str);
		} else if (!strcmp(str, "is-hidden")) {
			// return if the network is hidden (not broadcasting)
			if (router_opts.hidden_network) {
				strcpy(str, "Hidden Network");
			} else {
				strcpy(str, "Shown Network");
			}
			socketDoSend(s2, str);
		} else if (!strcmp(str, "show-network")) {
			// Broadcast essid
			syslog(LOG_NOTICE, "Sansgrid Daemon: Showing ESSID network");
			router_opts.hidden_network = 0;
			strcpy(str, "Showing Network");
			socketDoSend(s2, str);
		} else if (!strcmp(str, "pause")) {
			// pause packet sending
			if (!dispatch_pause) {
				strcpy(str, "Pausing payload handling");
				socketDoSend(s2, str);
			}
			dispatch_pause = 1;
		} else if (!strcmp(str, "resume")) {
			// resume packet sending
			if (dispatch_pause) {
				strcpy(str, "Resuming payload handling");
				socketDoSend(s2, str);
			}
			dispatch_pause = 0;
		}
		syslog(LOG_INFO, "sansgrid daemon: sending back: %s", str);

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

	getSansgridControlDir(socket_path);

	// Make sure the daemon is running 
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
	syslog(LOG_INFO, "sansgrid client: sending data");

	if (socketDoSend(s, data) == -1) {
		syslog(LOG_ERR, "sansgrid client: send error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Get the ACK back from the server
	while ((t = socketDoReceive(s, str)) > 0) {
		// check to see if the server got the kill message
		// Tell the user that the daemon is shutting down
		if (!strcmp(str, "kill")) {
			syslog(LOG_NOTICE, "sansgrid client: Shutting down daemon...\n");
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
	int error_code;

	getSansgridControlDir(config_path);
	snprintf(pidpath, 150, "%s/sansgridrouter.pid", config_path);
	if ((PIDFILE = fopen(pidpath, "w")) == NULL) {
		error_code = errno;
		syslog(LOG_ERR, "For path %s: %s", pidpath, strerror(error_code));
		perror("fopen");
		return -1;
	}
	printf("Running as process %i\n", pid);
	fprintf(PIDFILE, "%i\n", pid);
	fclose(PIDFILE);

	return 0;
}

int parseIPv6(char *ip_str, uint8_t ip_addr[16]) {
	uint8_t hexarray[16];
	uint8_t ip_right[16];
	char *divider;
	uint32_t size;
	uint32_t index,
			 base = 0;
	char *bounds;
	char *moved_ip;
	uint32_t right_size = 0;
	if (ip_str[0] == '\'')
		ip_str = &ip_str[1];
	if ((divider = strstr(ip_str, "\'")) != NULL) {
			divider[0] = '\0';
	}
	moved_ip = ip_str;
	if ((divider = strstr(ip_str, "::")) != NULL) {
		// found a divider
		right_size = parseIPv6(&divider[2], ip_right);
		divider[1] = '\0';
	} 
	bounds = &ip_str[strlen(ip_str)-1];
	memset(ip_addr, 0x0, 16);
	divider = moved_ip;

	while (divider < bounds) {
		if ((divider = strstr(moved_ip, ":")) == NULL) {
			divider = bounds;
		}
		if (divider[0] == ':')
			divider[0] = '\0';
		size = (strlen(moved_ip)+1)/2;
		atox(hexarray, moved_ip, sizeof(hexarray));
		for (index = base; index < base+size; index++) {
			ip_addr[index] = hexarray[index-base];
		}
		base = index;
		if (divider >= bounds)
			break;
		moved_ip = &divider[1];
	}
	for (index = 0; index < right_size; index++)
		ip_addr[16-right_size+index] = ip_right[index];

	return base;
}


int parseConfFile(const char *path, RouterOpts *ropts) {
	// parse a config file
	FILE *FPTR;
	char *buffer = NULL;
	size_t buff_alloc = 50;
	uint8_t netmask[IP_SIZE];
	char key[100],
		 url[100],
		 essid[100],
		 hidden_str[10],
		 verbosity_str[20],
		 netmask_str[50];
	int hidden = 0;
	int verbosity = 0;

	int foundkey = 0,
		foundurl = 0,
		foundessid = 0,
		foundhidden = 0,
		foundverbosity = 0,
		foundnetmask = 0;

	if ((FPTR = fopen(path, "r")) == NULL) {
		return -1;
	}
	buffer = (char*)malloc(buff_alloc*sizeof(char));
	if (buffer == NULL) {
		syslog(LOG_WARNING, "Couldn't allocate memory!");
		return -1;
	}
	while (getline(&buffer, &buff_alloc, FPTR) != -1) {
		if (strstr(buffer, "key")) {
			sscanf(buffer, "key = %s", key);
			foundkey = 1;
		} else if (strstr(buffer, "url")) {
			sscanf(buffer, "url = %s", url);
			foundurl = 1;
		} else if (strstr(buffer, "hidden")) {
			sscanf(buffer, "hidden = %s", hidden_str);
			foundhidden = 1;
			if (strstr(hidden_str, "1")) 
				hidden = 1;
			else if (strstr(hidden_str, "0"))
				hidden = 0;
			else
				foundhidden = 0;
		} else if (strstr(buffer, "essid")) {
			sscanf(buffer, "essid = %s", essid);
			foundessid = 1;
		} else if (strstr(buffer, "verbosity")) {
			sscanf(buffer, "verbosity = %s", verbosity_str);
			verbosity = atoi(verbosity_str);
			foundverbosity = 1;
		} else if (strstr(buffer, "netmask")) {
			sscanf(buffer, "netmask = %s", netmask_str);
			parseIPv6(netmask_str, netmask);
			foundnetmask = 1;
		}
	}
	fclose(FPTR);
	if (foundessid) 
		memcpy(ropts->essid, essid, sizeof(ropts->essid));
	if (foundhidden)
		ropts->hidden_network = hidden;
	if (foundurl)
		memcpy(ropts->serverip, url, sizeof(ropts->serverip));
	if (foundkey)
		memcpy(ropts->serverkey, key, sizeof(ropts->serverkey));
	if (foundverbosity) {
		ropts->verbosity = verbosity;
		setlogmask(LOG_UPTO(verbosity));
	}
	if (foundnetmask) {
		memcpy(ropts->netmask, netmask, IP_SIZE);
	}
		

	return 0;
}	


int main(int argc, char *argv[]) {
	pthread_t 	serial_read_thread,		// thread for reading over SPI
				dispatch_thread,		// thread for reading from dispatch
				heartbeat_thread,		// thread for pinging sensors
				fly_thread;				// thread for broadcasting ESSID

	int c;								// getopt var
	char *option = NULL;				// getopt var
	int32_t no_daemonize = 0;			// bool: should we run in foreground?
	char config_path[150];				// Sansgrid Dir
	pid_t sgpid;						// Sansgrid PID
	char payload[400];
	uint8_t ip_addr[IP_SIZE];
	SansgridHatching sg_hatch;
	SansgridSerial sg_serial;


	memset(&router_opts, 0x0, sizeof(RouterOpts));
	router_opts.verbosity = LOG_ERR;
	setlogmask(LOG_UPTO(router_opts.verbosity));

	getSansgridConfDir(config_path);
	strcat(config_path, "/sansgrid.conf");

	parseConfFile(config_path, &router_opts);

	// Parse arguments with getopt
	while (1) {
		const struct option long_options[] = {
			{"foreground",	no_argument, 		&no_daemonize, 	1},
			{"daemon", 		no_argument, 		&no_daemonize, 	0},
			{"config",      required_argument,  0,              'c'},
			{"packet",		required_argument, 	0,				'p'},
			{"verbose",     no_argument,        0,              'v'},
			{"quiet",       no_argument,        0,              'q'},
			{"help", 		no_argument, 		0, 				'h'},
			{"version", 	no_argument, 		0, 				0},
			{"serverip",	required_argument,	0,				's'},
			{"serverkey",	required_argument,	0,				'k'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "c:d:fhp:qvs:k:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				if (!strcmp(long_options[option_index].name, "version")) {
					printf("Not implemented yet!\n");
					exit(EXIT_FAILURE);
				}
				printf("option %s ", long_options[option_index].name);
				if (optarg)
					printf("With arg %s", optarg);
				printf("\n");
				break;
			case 'c':
				// User-supplied config file
				if (parseConfFile(optarg, &router_opts) == -1) {
					printf("Can't parse config file: %s\n", optarg);
					printf("Terminating\n");
					exit(EXIT_FAILURE);
				}
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
			case 'q':
				// Less verbosity
				if (--router_opts.verbosity < 0) {
					router_opts.verbosity = 0;
				}
				setlogmask(LOG_UPTO(router_opts.verbosity));
				break;
			case 's':
				// Server IP address given
				break;
			case 'v':
				// verbosity
				router_opts.verbosity++;
				setlogmask(LOG_UPTO(router_opts.verbosity));
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
		} else if (!strcmp(option, "is-hidden")) {
			// check to see if the network is hidden
			sgSocketSend("is-hidden", 10);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "url")) {
			// get the url
			sgSocketSend("url", 4);
			exit(EXIT_SUCCESS);
		} else if (strstr(option, "url=")) {
			// set the url
			sgSocketSend(option, strlen(option));
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "key")) {
			// get the key
			sgSocketSend("key", 4);
			exit(EXIT_SUCCESS);
		} else if (strstr(option, "key=")) {
			sgSocketSend(option, strlen(option));
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "drop")) {
			// drop a device
			if (optind < argc) {
				char doDrop[1000];
				sprintf(doDrop, "drop %s", argv[optind]);
				sgSocketSend(doDrop, strlen(doDrop));
				exit(EXIT_SUCCESS);
			}
		} else if (strstr(option, "packet=")) {
			sprintf(payload, "packet: %s", &option[7]);
			sgSocketSend(payload, strlen(payload));
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "running")) {
			// check to see if the daemon is running
			if ((sgpid = isRunning()) != 0) {
				printf("Running as process %i\n", sgpid);
			} else {
				printf("sansgridrouter is not running\n");
			}
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "pause")) {
			// pause sending packets
			sgSocketSend("pause", 6);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(option, "resume")) {
			// resume sending packets
			sgSocketSend("resume", 7);
			exit(EXIT_SUCCESS);
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

	// Initialize routing subsystem
	dispatch = queueInit(200);
	routing_table = routingTableInit(router_opts.netmask, "Stock ESSID");
	void *arg;

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
	pthread_create(&dispatch_thread, NULL, dispatchRuntime, dispatch);
	pthread_create(&heartbeat_thread, NULL, heartbeatRuntime, dispatch);
	pthread_create(&fly_thread, NULL, flyRuntime, dispatch);

	// Listen for commands or data from the server
	sgSocketListen();

	// Finished. Shut system down
	pthread_cancel(serial_read_thread);
	pthread_cancel(dispatch_thread);
	pthread_cancel(heartbeat_thread);
	pthread_cancel(fly_thread);

	pthread_join(serial_read_thread, &arg);
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
		printf("Try sansgridrouter -h\n");
	else {
		printf("Usage: sansgridrouter [OPTION]\n");
		printf("\n\
Startup Options\n\
  -c  --config=[CONFIGFILE]  use CONFIGFILE instead of default config file\n\
  -f  --foreground           Don't background daemon\n\
  -p  --packet=[PACKET]      Send a sansgrid payload to the server\n\
                             This arg is deprecated. Use the command instead.\n\
  -v  --verbose              Be verbose (warnings)\n\
  -vv                        Be more verbose (notices)\n\
  -vvv                       Be even more verbose (info)\n\
  -vvvv                      Be very very verbose (debug)\n\
  -q  --quiet                Be less verbose\n\
  -h, --help                 display this help and exit\n\
      --version              output version information and exit\n\
\n\
Daemon Querying\n\
      running                check to see if router daemon is running\n\
      devices                print number of devices tracked\n\
      status                 show status of devices\n\
\n\
Daemon Commands\n\
      packet=[PACKET]        Send a sansgrid payload to the server\n\
      start                  start the router daemon\n\
      restart                restart the router daemon\n\
      kill                   shutdown the router daemon\n\
      hide-network           don't broadcast essid\n\
      show-network           broadcast essid\n\
      drop [DEVICE]          drop a device\n\
      drop all               drop all devices\n\
      pause                  don't send any packets\n\
      resume                 continue sending packets\n\
\n\
Daemon Configuration\n\
      url                    get the server IP address\n\
      url=[SERVERIP]         set the server IP address\n\
      key                    get the server key\n\
      key=[SERVERKEY]        set the server key\n\
");
	}
	exit(status);
}



void getSansgridConfDir(char wd[150]) {
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

void getSansgridControlDir(char wd[150]) {
	// Get the location of the unix pipe and the .pid file
	struct stat buffer;
	int error_code;

	sprintf(wd, "/tmp/sansgrid");
	if (stat(wd, &buffer) >= 0) {
		// Found /tmp/sansgrid dir
		return;
	} else if (stat("/tmp", &buffer) >= 0) {
		// Found /tmp directory
		// Make sansgrid dir there
		if (mkdir(wd, 0777) < 0) {
			error_code = errno;
			syslog(LOG_ERR, "Couldn't create %s: %s", wd, strerror(error_code));
			exit(EXIT_FAILURE);
		} else {
			return;
		}
	} else {
		syslog(LOG_ERR, "Couldn't find /tmp dir. Exiting");
		exit(EXIT_FAILURE);
	}
	return;
}


// vim: ft=c ts=4 noet sw=4:
