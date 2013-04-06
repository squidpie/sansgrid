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


int main(int argc, char *argv[]) {
	pthread_t 	serial_read_thread,
				dispatch_thread;
	int c;
	int32_t no_daemonize = 0;
	while (1) {
		const struct option long_options[] = {
			{"foreground",	no_argument, 	&no_daemonize, 	1},
			{"daemon", 		no_argument, 	&no_daemonize, 	0},
			{"help", 		no_argument, 	0, 				'h'},
			{"version", 	no_argument, 	0, 				'v'},
			{0, 0, 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "fhv", long_options, &option_index);
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


	while (1) {
		sched_yield();
	}


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
  -h, --help                 display this help and exit\n\
  -v, --version              output version information and exit\n");
	}
	exit(status);
}

// vim: ft=c ts=4 noet sw=4:
