/* Router starting point
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


#ifndef __SG_ROUTER_MAIN_H__
#define __SG_ROUTER_MAIN_H__

// choose whether or not we should enforce packet order
#define SG_ROUTER_ENFORCE_PKT_ORD	1

#include <sys/types.h>
#include <unistd.h>

#include "dispatch/dispatch.h"
#include "routing_table/routing_table.h"
#include "daemon/sansgrid_daemon.h"

#define SG_SOCKET_BUFF_SIZE 1000

typedef struct RouterOpts {
	// options to pull out of a config file
	// or to grab from command-line arguments
	char essid[80];
	char serverip[20];
	char serverkey[512];
	uint8_t netmask[IP_SIZE];
	int hidden_network;		// whether or not to broadcast essid
	int verbosity;			// how loud we should be
	int strictness;
	int dispatch_pause;		// whether the dispatch should dequeue
	int heartbeat_period;	// how often a device should be pinged
} RouterOpts;


RouterOpts router_opts;
Queue *dispatch;
RoutingTable *routing_table;
uint8_t router_base[IP_SIZE];


int sgStorePID(pid_t pid);
void getSansgridConfDir(char wd[150]);
void getSansgridControlDir(char wd[150]);
int socketDoReceive(int s, char *str);
int socketDoSend(int s, const char *str);


#endif

// vim: ft=c ts=4 noet sw=4:
