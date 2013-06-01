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
/// \file

#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "dispatch/dispatch.h"
#include "routing_table/routing_table.h"
#include "daemon/sansgrid_daemon.h"

/// Socket Buffer size
#define SG_SOCKET_BUFF_SIZE 1000

/**
 * \brief Router Options Structure
 *
 * Storage for router options. These can be given at initialization as args,
 * or from a config file.
 */
typedef struct RouterOpts {
	// options to pull out of a config file
	// or to grab from command-line arguments
	/// Router ESSID
	char essid[80];
	/// IP Address for the server
	char serverip[20];
	/// Key to use for the server
	char serverkey[512];
	/// Base IP address
	uint8_t netmask[IP_SIZE];
	int hidden_network;		///< whether or not to broadcast essid
	int verbosity;			///< how loud we should be
	int strictness;			///< How strict authentication should be
	int dispatch_pause;		///< whether the dispatch should dequeue
	int dumping_dispatch;	///< whether or not to dump all packets
	int heartbeat_period;	///< how often a device should be pinged
	sem_t hb_wait;			///< post this to interrupt heartbeat sleep
} RouterOpts;


/// Router options instantiation
RouterOpts router_opts;
/// Global Ring buffer dispatch system
Queue *dispatch;
/// Global Routing Table 
RoutingTable *routing_table;


int sgStorePID(pid_t pid);
int getSansgridConfDir(char wd[150]);
void getSansgridControlDir(char wd[150]);
int socketDoReceive(int s, char *str);
int socketDoSend(int s, const char *str);


#endif

// vim: ft=c ts=4 noet sw=4:
