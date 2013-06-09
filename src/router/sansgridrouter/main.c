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
#include "routing_table/auth_status.h"

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
#include <time.h>
#include <semaphore.h>
/// \file


/**
 * \mainpage SansGrid Router: sansgridrouter
 *
 * \section intro_sec Introduction
 *
 * SansGrid is a distributed sensor infrastructure for the Internet of Things.
 * This is the router that communicates between a server and sensors.
 */

void usage(int status);
int spiSetup(void);
int spiTeardown(void);


/**
 * \brief Write/Action Thread
 *
 * This is one of two threads that is allowed to modify state in the router.
 * It takes a packet from the ring buffer dispatch, modifies internal
 * state appropriately, and forwards the payload toward its destination.
 *
 * \param[in,out]	arg		Not used
 */
void *dispatchRuntime(void *arg) {
	SansgridSerial *sg_serial = NULL;
	enum SansgridDeviceStatusEnum gen_ptype;
	int exit_code;

	while (1) {
		exit_code = 0;
		if(sg_serial != NULL) {
			free(sg_serial);
			sg_serial = NULL;
		}

		if (queueDequeue(dispatch, (void**)&sg_serial) == -1) {
			syslog(LOG_ERR, "Dispatch Queue Failed, Quitting");
			exit(EXIT_FAILURE);
		}
		while (router_opts.dispatch_pause) {
			sleep(1);
		}

		if (router_opts.dumping_dispatch) {
			// Remove everything from the dispatch
			do {
				if (sg_serial) {
					free(sg_serial);
					sg_serial = NULL;
				}
				queueDequeue(dispatch, (void**)&sg_serial);
			} while (queueSize(dispatch) > 0);
			router_opts.dumping_dispatch = 0;
			continue;
		}

		printf("Got packet of type %x\n", sg_serial->payload[0]);
		if (sg_serial->payload[0] == SG_SERVSTATUS) {
			routerHandleServerStatus(routing_table, sg_serial);
		} else {
			gen_ptype = sgPayloadGetType(sg_serial->payload[0]);
			switch (gen_ptype) {
				case SG_DEVSTATUS_HATCHING:
					exit_code = routerHandleHatching(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_FLYING:
					exit_code = routerHandleFly(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_EYEBALLING:
					exit_code = routerHandleEyeball(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_PECKING:
					exit_code = routerHandlePeck(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_SINGING:
					exit_code = routerHandleSing(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_MOCKING:
					exit_code = routerHandleMock(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_PEACOCKING:
					exit_code = routerHandlePeacock(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_NESTING:
					exit_code = routerHandleNest(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_SQUAWKING:
					exit_code = routerHandleSquawk(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_HEARTBEAT:
					exit_code = routerHandleHeartbeat(routing_table, sg_serial);
					break;
				case SG_DEVSTATUS_CHIRPING:
					exit_code = routerHandleChirp(routing_table, sg_serial);
					break;
				default:
					printf("Not found: %x->%x\n", 
							sg_serial->payload[0], gen_ptype);
					break;
			}
		}
		if (exit_code == -1) {
			if (routingTableIsAuthStrict(routing_table) == DEV_AUTH_STRICT) 
				routerFreeDevice(routing_table, sg_serial->ip_addr);
		}
	}
	pthread_exit(arg);
}


/**
 * \brief SPI Listener/Receive Thread
 *
 * This thread listens on the serial line and enqueues data received.
 *
 * \param[in,out]	arg		Not used
 */
void *spiReaderRuntime(void *arg) {
	// Read from SPI and queue data onto dispatch
	uint32_t size;
	SansgridSerial *sg_serial;
	while (1) {
		while (sgSerialReceive(&sg_serial, &size) == -1) {
			free(sg_serial);
			sched_yield();
		}
		//queueEnqueue(dispatch, sg_serial);
	}
	pthread_exit(arg);
}


/**
 * \brief Heartbeat Thread
 *
 * This thread sends out a heartbeat to devices periodically
 * \param[in,out]	arg		Not used
 */
void *heartbeatRuntime(void *arg) {
	// handle pings

	int32_t count;
	uint8_t ip_addr[IP_SIZE];
	SansgridSerial *sg_serial = NULL;
	SansgridHeartbeat sg_hb;
	SansgridIRStatus sg_irstatus;
	SansgridChirp sg_chirp;
	struct timespec req, epoch;
	int hb_status = 0;
	uint32_t rdid;
	int device_lost = 0;
	uint32_t current_packet;
	uint64_t nano_add = 0;
	uint64_t sec_add = 0;
	int oldstatus;

	// We have to make sure this thread can be cancelled whenever.
	// Otherwise we may miss the signal
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstatus);

	memset(&sg_irstatus, 0x0, sizeof(SansgridIRStatus));
	memset(&sg_chirp, 0x0, sizeof(SansgridChirp));
	memset(&sg_hb, 0x0, sizeof(SansgridHeartbeat));
	sg_hb.datatype = SG_HEARTBEAT_ROUTER_TO_SENSOR;
	routingTableForEachDevice(routing_table, ip_addr);
	count = routingTableGetDeviceCount(routing_table)-1;
	while (1) {
		do {
			clock_gettime(CLOCK_REALTIME, &epoch);
			if (count == 0)
				count = 1;
			if (router_opts.heartbeat_period/count == 0) {
				// interval is < 1 second
				// sleep in usecs
				req.tv_nsec = ((router_opts.heartbeat_period*1000L)/count)*1000000L;
				req.tv_sec = 0;
			} else {
				req.tv_nsec = 0;
				req.tv_sec = router_opts.heartbeat_period / count;
			}
			nano_add = epoch.tv_nsec + req.tv_nsec;
			sec_add = epoch.tv_sec + req.tv_sec;
			if (nano_add > 1000000000L) {
				// wrap nanoseconds into seconds
				nano_add = (nano_add-1000000000L);
				sec_add++;
			}
			req.tv_nsec = nano_add;
			req.tv_sec = sec_add;
			
			sem_timedwait(&router_opts.hb_wait, &req);
		} while ((count = (routingTableGetDeviceCount(routing_table)-1)) < 1);

		while (router_opts.dispatch_pause) {
			sleep(1);
		}

		current_packet = routingTableGetCurrentPacket(routing_table, ip_addr);
		if ((current_packet != SG_DEVSTATUS_NESTING)
				&& (current_packet != SG_DEVSTATUS_CHIRPING)) {
			char name[100];
			sgPayloadGetPayloadName(current_packet, name);
			// device is still authenticating
			// continue on
			if (routingTableStepNextDevice(routing_table, ip_addr) == 0) {
				routingTableForEachDevice(routing_table, ip_addr);
			}
			continue;
		}

		if (!routingTableIsDeviceLost(routing_table, ip_addr)) {
			syslog(LOG_DEBUG, "heartbeat: sending to device %u", routingTableIPToRDID(routing_table, ip_addr));
			sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
			memcpy(sg_serial->payload, &sg_hb, sizeof(SansgridHeartbeat));
			memcpy(sg_serial->ip_addr, ip_addr, IP_SIZE);
			queueEnqueue(dispatch, sg_serial);
			sg_serial = NULL;
		}
		if ((hb_status = routingTableHeartbeatDevice(routing_table, ip_addr)) != 0) {
			// device status changed... either went stale or was lost
			printf("hb_status = %x\n", hb_status);
			sg_irstatus.datatype = SG_SERVSTATUS;
			rdid = routingTableIPToRDID(routing_table, ip_addr);
			wordToByte(sg_irstatus.rdid, &rdid, sizeof(rdid));
			if (routingTableIsDeviceLost(routing_table, ip_addr)) {
				// Device was just lost
				syslog(LOG_NOTICE, "Device %i has just been lost", routingTableIPToRDID(routing_table, ip_addr));
				strcpy(sg_irstatus.status, "lost");
				device_lost = 1;
			} else if (routingTableIsDeviceStale(routing_table, ip_addr)) {
				// Device just went stale
				strcpy(sg_irstatus.status, "stale");
				syslog(LOG_NOTICE, "Device %i has just gone stale", routingTableIPToRDID(routing_table, ip_addr));
			}
			sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
			sg_serial->control = SG_SERIAL_CTRL_VALID_DATA;
			memcpy(sg_serial->payload, &sg_irstatus, sizeof(SansgridIRStatus));
			memcpy(sg_serial->ip_addr, ip_addr, IP_SIZE);
			queueEnqueue(dispatch, sg_serial);
			sg_serial = NULL;
			if (device_lost) {
				device_lost = 0;
				routerFreeDevice(routing_table, ip_addr);
			}
		}
		if (routingTableStepNextDevice(routing_table, ip_addr) == 0) {
			routingTableForEachDevice(routing_table, ip_addr);
		}
	}

	pthread_exit(arg);
}


/** \brief Sends out a fly ESSID periodically
 *
 * \param[in,out]	arg		Not used
 */
void *flyRuntime(void *arg) {
	// handle broadcast of ESSID
	SansgridFly sg_fly;
	SansgridSerial *sg_serial = NULL;
	memset(&sg_fly, 0x0, sizeof(SansgridFly));
	sg_fly.datatype = SG_FLY;
	while (1) {
		if (!router_opts.hidden_network) {
			sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
			memset(sg_serial, 0x0, sizeof(SansgridSerial));
			routingTableGetEssid(routing_table, sg_fly.network_name);
			//for(uint32_t i=strlen(sg_fly.network_name); i<sizeof(sg_fly.network_name); i++)
			//	sg_fly.network_name[i] = i;
			memcpy(sg_serial->payload, &sg_fly, sizeof(SansgridFly));
			sg_serial->control = SG_SERIAL_CTRL_VALID_DATA;
			routingTableGetBroadcast(routing_table, sg_serial->ip_addr);
			while (router_opts.dispatch_pause) {
				sleep(1);
			}	
			queueEnqueue(dispatch, sg_serial);
			sg_serial = NULL;
			//routerHandleFly(routing_table, &sg_serial);
		}
		sleep(13);
	}

	pthread_exit(arg);
}
	


/**
 * \brief Convenience function to send data to a socket
 *
 * \param[in]	s	an open socket
 * \param[in]	str	null-terminated string
 * \returns
 * On success, return 0. \n
 * Otherwise (on error), return 0
 */
int socketDoSend(int s, const char *str) {
	if (send(s, str, strlen(str), 0) == -1) {
		return -1;
	} else {
		return 0;
	}
}



/**
 * \brief Convenience funtion to receive data on a socket
 *
 * \param[in]	s	an open socket
 * \param[out]	str	a null-terminated string
 *
 * This function also strips off trailing newlines.
 *
 * \returns
 * On success, this function returns 0
 * Otherwise, on error, returns -1
 */
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



/**
 * \brief Create a socket and send data to a receiver
 *
 * \param[in]	data	A null-terminated string to send
 * \param[in]	size	Size of the data to send
 *
 * \returns
 * On success, return 0
 * On failure, return -1
 */
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
		printf("%s\n", str);
	}
	// cleanup
	close(s);

	return 0;
}



/**
 * \brief Store the PID of the running process
 */
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



/**
 * \brief Extract an IPv6 address from a null-terminated string
 *
 * \param[in]	ip_str	null-terminated string containing an IP address
 * \param[out]	ip_addr	IP address
 */
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
		atox(hexarray, moved_ip, size);
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


static char *parseOption(char *buffer) {
	char *out;
	for (out = buffer; *out != '\0'; out++) {
		if (out[0] == '=') break;
	}
	if (*out == '\0') return out;

	for (out++; *out != '\0'; out++) {
		if (out[0] != ' ') break;
	}
	if (out[strlen(out)-1] == '\n')
		out[strlen(out)-1] = '\0';
	return out;
}



static char *parseWithQuotes(char *buffer) {
	// parse with a value in single quotes
	char *str;
	char *saveptr;
	str = strtok_r(buffer, "'", &saveptr);
	str = strtok_r(NULL, "'", &saveptr);
	if (str[strlen(str)-1] == '\n')
		str[strlen(str)-1] = '\0';
	return str;
}

/**
 * \brief Get configuration from a Config File
 */
int parseConfFile(const char *path, RouterOpts *ropts) {
	// parse a config file
	FILE *FPTR;
	char *buffer = NULL;
	size_t buff_alloc = 50;
	uint8_t netmask[IP_SIZE];
	char key[100],
		 url[100],
		 essid[100],
		 strictness_str[10];
	int hidden = 0;
	int verbosity = 0;
	int strictness = 0;
	int heartbeat = 0;

	int foundkey = 0,
		foundurl = 0,
		foundessid = 0,
		foundhidden = 0,
		foundverbosity = 0,
		foundnetmask = 0,
		foundstrictness = 0,
		foundheartbeat = 0;
	char *str = NULL;

	if ((FPTR = fopen(path, "r")) == NULL) {
		return -1;
	}
	buffer = (char*)malloc(buff_alloc*sizeof(char));
	if (buffer == NULL) {
		syslog(LOG_WARNING, "Couldn't allocate memory!");
		return -1;
	}
	while (getline(&buffer, &buff_alloc, FPTR) != -1) {
		if (strstr(buffer, "\'")) {
			str = parseWithQuotes(buffer);
		} else {
			str = parseOption(buffer);
		}
		if (strstr(buffer, "key")) {
			strncpy(key, str, sizeof(key));
			foundkey = 1;
		} else if (strstr(buffer, "url")) {
			strncpy(url, str, sizeof(url));
			foundurl = 1;
		} else if (strstr(buffer, "hidden")) {
			foundhidden = 1;
			if (strstr(str, "1")) 
				hidden = 1;
			else if (strstr(str, "0"))
				hidden = 0;
			else
				foundhidden = 0;
		} else if (strstr(buffer, "strictness")) {
			strncpy(strictness_str, str, sizeof(strictness_str));
			foundstrictness = 1;
			if (strstr(strictness_str, "strict"))
				strictness = DEV_AUTH_STRICT;
			else if (strstr(strictness_str, "filtered"))
				strictness = DEV_AUTH_FILTERED;
			else if (strstr(strictness_str, "loose"))
				strictness = DEV_AUTH_LOOSE;
			else
				foundstrictness = 0;
		} else if (strstr(buffer, "essid")) {
			strcpy(essid, str);
			foundessid = 1;
		} else if (strstr(buffer, "verbosity")) {
			verbosity = atoi(str);
			foundverbosity = 1;
		} else if (strstr(buffer, "netmask")) {
			parseIPv6(str, netmask);
			foundnetmask = 1;
		} else if (strstr(buffer, "heartbeat")) {
			heartbeat = atoi(str);
			foundheartbeat = 1;
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
	if (foundnetmask)
		memcpy(ropts->netmask, netmask, IP_SIZE);
	if (foundstrictness) {
		ropts->strictness = strictness;
	}
	if (foundheartbeat)
		ropts->heartbeat_period = heartbeat;

	return 0;
}	



/**
 * \brief Sansgrid Router Startup point
 */
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
	SansgridSerial *sg_serial;


	memset(&router_opts, 0x0, sizeof(RouterOpts));
	router_opts.dispatch_pause = 0;
	strcpy(router_opts.essid, "SansGrid");
	router_opts.heartbeat_period = 10;
	router_opts.hidden_network = 0;
	router_opts.netmask[IP_SIZE-1] = 0x1;
	strcpy(router_opts.serverip, "127.0.0.1");
	router_opts.strictness = DEV_AUTH_LOOSE;
	router_opts.verbosity = LOG_ERR;
	setlogmask(LOG_UPTO(router_opts.verbosity));
	sem_init(&router_opts.hb_wait, 0, 0);

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
					printf("sansgridrouter (SansGrid) 1.00\n\n");
					printf("This is free software: you are free to change and redistribute it.\n");
					printf("There is NO WARRANTY, to the extent permitted by law.\n");
					exit(EXIT_SUCCESS);
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

	// Stock commands that don't require extra parsing to send
	char *commands[] = {
		"kill",
		"status",
		"devices",
		"is-hidden",
		"url", "url=",
		"key", "key=",
		"heartbeat=",
		"auth=",
		"network=",
		"packet=", "packet:",
		"pause", "resume",
	};

	// Parse remaining commands
	while (optind < argc) {
		// deal with non-option argv elements
		// Do some checking so we don't have to bother the daemon as much
		option = argv[optind++];
		if (!strcmp(option, "start")) {
			// daemonize
			no_daemonize = 0;
		} else if (!strcmp(option, "restart")) {
			// kill and start daemon
			sgSocketSend("kill", 4);
			sleep(1);
			no_daemonize = 0;
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
		} else {
			int sizecap;
			for (uint32_t i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
				if (!strncmp(commands[i], option, strlen(commands[i]))) {
					sizecap = strlen(option);
					if (sizecap > SG_SOCKET_BUFF_SIZE) {
						sizecap = SG_SOCKET_BUFF_SIZE-1;
					}
					sgSocketSend(option, sizecap);
					exit(EXIT_SUCCESS);
				}
			}
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
	routing_table = routingTableInit(router_opts.netmask, router_opts.essid);
	void *arg;

	switch(router_opts.strictness) {
		case DEV_AUTH_LOOSE:
			routingTableAllowLooseAuth(routing_table);
			break;
		case DEV_AUTH_FILTERED:
			routingTableSetAuthFiltered(routing_table);
			break;
		case DEV_AUTH_STRICT:
			routingTableRequireStrictAuth(routing_table);
			break;
		default:
			routingTableSetAuthFiltered(routing_table);
			break;
	}

	if (spiSetup() < 0) {
		syslog(LOG_ERR, "Couldn't initialize SPI");
		exit(EXIT_FAILURE);
	}

	sg_serial = (SansgridSerial*)malloc(sizeof(SansgridSerial));
	memset(&sg_hatch, 0x0, sizeof(SansgridHatching));
	memset(sg_serial, 0x0, sizeof(SansgridSerial));
	routingTableGetRouterIP(routing_table, ip_addr);
	//memset(ip_addr, 0x0, IP_SIZE);
	//ip_addr[IP_SIZE-1] = 0x1;
	sg_hatch.datatype = SG_HATCH;
	memcpy(sg_hatch.ip, ip_addr, IP_SIZE);
	memcpy(sg_serial->payload, &sg_hatch, sizeof(SansgridHatching));
	sg_serial->control = SG_SERIAL_CTRL_VALID_DATA;
	memcpy(sg_serial->ip_addr, ip_addr, IP_SIZE);
	queueEnqueue(dispatch, sg_serial);


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
	sem_destroy(&router_opts.hb_wait);
	queueDestroy(dispatch);
	routingTableDestroy(routing_table);
	spiTeardown();

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
      drop [DEVICE]          drop a device\n\
      drop all               drop all devices\n\
      drop queue             drop all enqueued items\n\
      pause                  don't send any packets\n\
      resume                 continue sending packets\n\
\n\
Daemon Configuration\n\
      auth=                  control adherence to auth protocol at router\n\
           loose               device must eyeball (default)\n\
           filtered            unexpected packets are dropped, devices stay\n\
           strict              device must follow exact protocol or is dropped\n\
      network={hidden,shown} control sending of essid packet\n\
      url                    get the server IP address\n\
      url=[SERVERIP]         set the server IP address\n\
      key                    get the server key\n\
      key=[SERVERKEY]        set the server key\n\
      heartbeat=[PERIOD]     set the heartbeat period\n\
");
	}
	exit(status);
}



/**
 * \brief Get the Sansgrid Configuration Directory
 */
int getSansgridConfDir(char wd[150]) {
	// Get the .sansgrid directory path
	// Return success or failure
	// pass the path back in wd
	char *home_path = getenv("HOME");
	struct stat buffer;

	if (home_path) {
		snprintf(wd, 120, "%s/.sansgrid", home_path);
		if (stat(wd, &buffer) >= 0) {
			// found an existing dir
			return 0;
		}
	} 
	// didn't find sansgrid dir in $HOME 
	// or couln't find $HOME
	// Try /etc/sansgrid
	sprintf(wd, "/etc/sansgrid");
	if (stat(wd, &buffer) >= 0) {
		// found dir in /etc
		return 0;
	} else {
		return -1;
	}
}



/**
 * \brief Get the Sansgrid Control Directory
 */
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
