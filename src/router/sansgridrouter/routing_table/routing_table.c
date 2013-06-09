/* A quick implementation for a statically-defined table.
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
 * This implementation defines the table size at compile-time instead of at run-time,
 * making the system less prone to bugs. The table is an array of pointers, which
 * facilitates creating/moving/deleting IP addresses.
 */


#include <stdio.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "routing_table.h"
#include "auth_status.h"
#include "heartbeat.h"
#include "../payload_handlers/payload_handlers.h"
/// \file




/**
 * \brief Tracking for an authenticating/authenticated device
 *
 * This is a node that tracks information on a device
 * that is authenticating, or has been authenticated.
 * It contains heartbeat information, and authentication status.
 */
typedef struct RoutingNode {
	/** 
	 * \brief Device Identifier
	 * 
	 * This is the identifier that the server uses to track devices.
	 * Currently, due to how a device requests connection to the network,
	 * it very closely corresponds to the device IP address.
	 */
	uint32_t rdid;
	/**
	 * \brief Device health
	 *
	 * Tracks how long ago the device was heard from
	 */
	HeartbeatStatus *hb;
	/**
	 * \brief Device Authentication Status
	 *
	 * Tracks the place the device is in the authentication path,
	 * what the next expected packet type is, and the device's
	 * strictness.
	 */
	DeviceAuth *auth;
} RoutingNode;
	

/**
 * \brief Routing Table Structure
 *
 * The routing table tracks connected devices.
 */
struct RoutingTable {
	/// Index, used for alloc
	uint32_t tableptr;			
	/// For each index, used for stepping through nodes
	uint32_t feach_index;		
	/// Identifier pool
	uint32_t rdid_pool;			
	/// Number of devices being tracked
	uint32_t table_alloc;
	/// The table of devices
	RoutingNode *routing_table[ROUTING_ARRAYSIZE];
	/// What strictness a device is assigned when it is allocated
	int default_strictness;	

	/// Network name that is sent out on a fly
	char essid[80];
	
	/// IP submask partition
	uint8_t base[IP_SIZE];
	/// Broadcast IP address
	uint8_t broadcast[IP_SIZE];
};



/**
 * \brief Make sure the table is non-null
 *
 * This makes sure the table has a nonnull address.
 * If the table isn't allocated, the program exits 
 * with EXIT_FAILURE status
 */
static int tableAssertValid(RoutingTable *table) {
	// Exit if the table isn't allocated
	if (table == NULL) {
		syslog(LOG_ERR, "routing table not initialized; quitting");
		exit(EXIT_FAILURE);
	}
	return 0;
}


/**
 * \brief Check to see if the table is full
 *
 * If no more devices can be allocated, return 1. \n
 * Otherwise return 0.
 */
static int tableChkFull(RoutingTable *table) {
	// Return 1 if table is full, 0 if not
	if (table->table_alloc >= ROUTING_ARRAYSIZE-3) {
		syslog(LOG_NOTICE, "routing table full");
		return 1;
	}
	return 0;
}


/**
 * \brief Check to see if the table is empty
 *
 * If no devices are allocated, return 1. \n
 * Otherwise return 0.
 */
static int tableChkEmpty(RoutingTable *table) {
	if (table->table_alloc < 2) {
		syslog(LOG_NOTICE, "routing table empty");
		return 1;
	}
	return 0;
}



/**
 * \brief Mask the table pointer with a given base to make the IP address
 * \param[out]	ip_addr		Constructed IP address
 * \param[in]	base		Netmask to use
 * \param[in]	tableptr	Table Index of device
 */
static int maskip(uint8_t ip_addr[IP_SIZE], 
		uint8_t base[IP_SIZE], 
		uint32_t tableptr) {
	// Mask the tableptr with the base to make an ip address
	
	int i, ioffset;
	uint8_t tablebyteptr[4];

	wordToByte(tablebyteptr, &tableptr, 1*sizeof(uint32_t));

	memcpy(ip_addr, base, IP_SIZE*sizeof(uint8_t));
	for (i=0; i<4; i++) {
		ioffset = IP_SIZE-(4-i);
		ip_addr[ioffset] = base[ioffset] | tablebyteptr[i];
	}

	return 0;
}



/**
 * \brief Take an IP address and return where it should be in the routing table
 * \param[in]	ip_addr		IP address of device to find
 * \param[in]	base		Netmask to use
 * \returns
 * The location the device would be in
 */
static uint32_t locationToTablePtr(uint8_t ip_addr[IP_SIZE], uint8_t base[IP_SIZE]) {
	// Take a location and show where it should be in the routing table

	int32_t i;
	uint8_t offset[IP_SIZE];
	uint32_t location[IP_SIZE/4];
	uint32_t index;

	for (i=0; i<IP_SIZE; i++)
		offset[i] = ip_addr[i] - base[i];
	byteToWord(location, offset, IP_SIZE*sizeof(uint8_t));

	index = location[IP_SIZE/4-1];
	if (index >= ROUTING_ARRAYSIZE)
		return 0;

	return index;
}


/**
 * \brief Converts an array of words to an array of bytes
 *
 * This function converts an array containing uint32_t members
 * into an array of uint8_t, respecting endianness.
 * This is an unsafe function. It does no bounds checking.
 * bytes[] isn't size-checked. it must be 4x the size of wordsize.
 * \param[out]	bytes		A returned array of bytes
 * \param[in]	words		An array of words to use
 * \param[in]	bytesize	Number of bytes to convert
 */
void wordToByte(uint8_t *bytes, uint32_t *words, size_t bytesize) {
	// Convert an array of words into an array of bytes

	uint32_t i;
	uint32_t endianconv;
	size_t wordsize = (bytesize * sizeof(uint8_t)) / sizeof(uint32_t);

	for (i=0; i<wordsize; i++) {
		endianconv = htonl(words[i]);
		memcpy(&bytes[4*i], &endianconv, sizeof(uint32_t));
	}

	return;
}



/** \brief converts an array of bytes into an array of words
 * 
 * This function converts an array containing uint8_t bytes
 * into an array of uint32_t, respecting endianness.
 * This is an unsafe function. It does no bounds checking.
 * bytesize must be a multiple of 4.
 * words[] isn't size-checked. it must be >= (bytesize/4)+1.
 * \param[out]	words		An array of words taken from an array of bytes
 * \param[in]	bytes		An array of bytes to use
 * \param[in]	bytesize	Number of bytes to convert
 * \returns
 * If bytesize isn't a multiple of 4, 
 * the conversion isn't done and -1 is returned. \n
 * Otherwise return 0
 */
int byteToWord(uint32_t *words, uint8_t *bytes, size_t bytesize) {
	// Convert an array of words into an array of bytes
	
	uint32_t i;
	uint32_t endianconv;

	if (bytesize % 4)
		return -1;

	for (i=0; i<(bytesize/4); i++) {
		memcpy(&endianconv, &bytes[i*4], sizeof(uint32_t));
		words[i] = ntohl(endianconv);
	}

	return 0;
}



/** \brief Initialize the Routing Table
 *
 * Initializes the routing table with a base subnet and an essid.
 * \param[in]	base	Netmask to use
 * \param[in]	essid	Network ESSID to assign
 * \returns
 * On success, a pointer to a new routing table structure is returned.
 */
RoutingTable *routingTableInit(uint8_t base[IP_SIZE], char essid[80]) {
	// Initialize the routing table
	
	int i;
	RoutingTable *table;
	int unmasked_bits = ROUTING_UNIQUE_BITS;
	int maskindex = IP_SIZE-1;
	uint8_t mask;

	table = (RoutingTable*)malloc(sizeof(RoutingTable));
	table->rdid_pool = 1;
	table->tableptr = 0;
	table->table_alloc = 0;
	table->default_strictness = 1;
	while (unmasked_bits > 0) {
		if (unmasked_bits >= 8) {
			mask = 0xff;
			unmasked_bits -= 8;
		} else {
			mask = (1 << unmasked_bits)-1;
			unmasked_bits = 0;
		}
		base[maskindex] = base[maskindex] & ~mask;
		maskindex--;
	}
	memcpy(table->base, base, IP_SIZE*sizeof(uint8_t));

	memset(table->broadcast, 0xff, IP_SIZE*sizeof(uint8_t));
	strncpy(table->essid, essid, sizeof(table->essid));
	// TODO: Check to make sure lowest ROUTING_UNIQUE_BITS are 0
	

	for (i=0; i<ROUTING_ARRAYSIZE; i++)
		table->routing_table[i] = NULL;

	return table;
}



/**
 * \brief Free all resources from the routing table
 *
 * Note that this does not send any disconnect signals.
 * For that you want to use [routerFreeDevice](@ref routerFreeDevice)
 */
RoutingTable *routingTableDestroy(RoutingTable *table) {
	// Free any memory associated with the routing table
	
	int i;
	uint8_t ip_addr[IP_SIZE];

	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			maskip(ip_addr, table->base, i);
			routingTableFreeIP(table, ip_addr);
		}
	}

	free(table);
	table = NULL;
	return table;
}

/**
 * \brief Get the network ESSID
 *
 * Stores the network ESSID in essid
 */
void routingTableGetEssid(RoutingTable *table, char essid[80]) {
	// Get the ESSID from the routing table

	tableAssertValid(table);
	strncpy(essid, table->essid, sizeof(table->essid));
	return;
}


/**
 * \brief Try to assign ip_addr
 *
 * This will statically assign ip_addr IP address if possible.
 * \n
 * \returns
 * If successful, return 0. Otherwise return -1.
 */
int32_t routingTableAssignIPStatic(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Statically assign IP Address if possible
	// return 0 if success, -1 if failure
	
	uint32_t rdid_pool;
	int32_t index;

	tableAssertValid(table);
	if (tableChkFull(table))
		return 1;

	index = locationToTablePtr(ip_addr, table->base) % ROUTING_ARRAYSIZE;
	if (index < 1 || index == (ROUTING_ARRAYSIZE-1))
		return 1;

	if (routingTableLookup(table, ip_addr) == 0) {
		// Allocate space for the device
		syslog(LOG_INFO, "Allocating slot for new device at %i", index);
		rdid_pool = table->rdid_pool;
		table->routing_table[index] = (RoutingNode*)malloc(sizeof(RoutingNode));
		if (!routingTableLookupRDID(table, index)) {
			table->routing_table[index]->rdid = index;
		} else {
			while (routingTableLookupRDID(table, rdid_pool)) {
				rdid_pool++;
			}
			table->routing_table[index]->rdid = rdid_pool++;
			table->rdid_pool = rdid_pool;
		}
		syslog(LOG_NOTICE, "New device with rdid %u entering network", 
				table->routing_table[index]->rdid);

		table->routing_table[index]->hb = hbInitDefault();
		table->routing_table[index]->auth = deviceAuthInit(table->default_strictness);
		table->table_alloc++;
		return 0;
	} else {
		syslog(LOG_INFO, "Device already exists at %i", index);
		return 1;
	}
}



/**
 * \brief Assign an IP
 *
 * \param[in,out]	ip_addr		A requested IP address, also contains the actual allocated IP address
 * \returns 0 if successful, -1 on failure
 */
int32_t routingTableAssignIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Allocate the next available block and give it an IP address

	uint32_t tableptr;


	tableAssertValid(table);
	if (tableChkFull(table))
		return 1;

	tableptr = table->tableptr;

	// Find next available slot
	while (table->routing_table[tableptr] 
			|| tableptr < 2
			|| tableptr == (ROUTING_ARRAYSIZE-1)) {
		tableptr = (tableptr + 1) % ROUTING_ARRAYSIZE;
	}
	maskip(ip_addr, table->base, tableptr);	// create IP address
	table->tableptr = tableptr;

	// Allocate space for the device
	return routingTableAssignIPStatic(table, ip_addr);
}



/**
 * \brief Remove ip_addr from the table
 *
 * \param[in]     ip_addr		IP address to remove
 * \returns
 * On successful removal, return 0.
 * \n
 * Otherwise return -1.
 */
int32_t routingTableFreeIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Release IP Address
	
	uint32_t index;


	tableAssertValid(table);
	if (tableChkEmpty(table))
		return -1;

	// table lookup
	index = locationToTablePtr(ip_addr, table->base);

	if (index >= ROUTING_ARRAYSIZE || index < 2)
		return -1;
	if (table->routing_table[index] == NULL)
		return -1;

	// Release slot
	//free(table->routing_table[index]->properties);
	free(table->routing_table[index]);
	table->routing_table[index] = NULL;
	table->table_alloc--;

	return 0;
}


/**
 * \brief Remove all devices from the table
 *
 * Removes all IP addresses from the table, except for the router entry.
 * \returns
 * always 0
 */
int32_t routingTableFreeAllIPs(RoutingTable *table) {
	// Release all IP addresses
	
	uint32_t i;
	uint8_t ip_addr[IP_SIZE];

	tableAssertValid(table);

	for (i=2; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			maskip(ip_addr, table->base, i);
			routingTableFreeIP(table, ip_addr);
		}
	}

	return 0;
}



/**
 * \brief Check for existence of RDID in Routing Table
 *
 * Queries the routing table for existence of rdid
 * \returns
 * 1 if a match is found \n
 * 0 if a match is not found
 */
int32_t routingTableLookupRDID(RoutingTable *table, uint32_t rdid) {
	// Lookup an RDID in the table
	// return true if found, false if not found
	
	uint32_t i;
	tableAssertValid(table);
	if (!table->table_alloc)
		return 0;

	// table lookup
	
	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			if (table->routing_table[i]->rdid == rdid) {
				return 1;
			}
		}
	}

	return 0;
}



/**
 * \brief Check for existence of ip_addr in Routing Table
 *
 * Queries the routing table for existence of IP address ip_addr
 * \returns
 * 1 if a match is found \n
 * 0 if a match is not found
 */
int32_t routingTableLookup(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Lookup an IP Address in the table
	// return true if found, false if not found
	
	uint32_t index;


	tableAssertValid(table);
	if (!table->table_alloc)
		return 0;

	// table lookup
	index = locationToTablePtr(ip_addr, table->base);

	if (index >= ROUTING_ARRAYSIZE)
		return 0;

	if (table->routing_table[index] == NULL)
		return 0;

	return 1;
}



/**
 * \brief Converts an IP address to an rdid
 *
 * Looks up ip_addr in table and, if found, returns the rdid of that device.
 * \returns
 * Matching device rdid if found \n
 * 0 if device is not found
 */
uint32_t routingTableIPToRDID(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Lookup an IP address in the table
	// return a unique identifier if found,
	// return 0 if false
	
	int index;
	if (routingTableLookup(table, ip_addr) == 0) {
		// device not found
		return 0;
	}

	index = locationToTablePtr(ip_addr, table->base);
	return table->routing_table[index]->rdid;
}



/**
 * \brief Converts an rdid into an IP address
 * \param[in]	rdid		device rdid to lookup
 * \param[out]	ip_addr		matched device IP address
 * \returns
 * 0 if device was matched \n
 * -1 if device was not matched
 */
int32_t routingTableRDIDToIP(RoutingTable *table, 
		uint32_t rdid, 
		uint8_t ip_addr[IP_SIZE]) {
	// convert an rdid to an IP address
	for (uint32_t i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			if (table->routing_table[i]->rdid == rdid) {
				maskip(ip_addr, table->base, i);
				return 0;
			}
		}
	}
	maskip(ip_addr, table->base, rdid&0xff);
	return -1;
}



/**
 * \brief Gets Broadcast IP for the routing table
 *
 * Note that IPv6 does not normally do broadcast. \n
 * This is akin to Ethernet's Broadcast address, so the packet is 
 * sent to all connected devices.
 * \param[out]	broadcast		The Routing Table's Broadcast IP address
 */
void routingTableGetBroadcast(RoutingTable *table, uint8_t broadcast[IP_SIZE]) {
	// get the broadcast address
	memcpy(broadcast, table->broadcast, IP_SIZE);
	return;
}



/**
 * \brief Returns the Router's IP address
 *
 * \param[out]	ip_addr			The Router's IP address
 */
void routingTableGetRouterIP(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Get the router's IP address
	maskip(ip_addr, table->base, 1);
	return;
}



/**
 * \brief Check to see if this is a valid packet to get from this IP address
 *
 * \param		ip_addr			The device IP address
 * \param		dt				The Sansgrid Packet datatype
 * \returns
 * 1 if valid. \n
 * 0 if invalid.
 */
int32_t routingTableCheckValidPacket(RoutingTable *table, uint8_t ip_addr[IP_SIZE],
		enum SansgridDataTypeEnum dt) {
	// check to see if this datatype is valid
	tableAssertValid(table);

	if (!table->table_alloc)
		return SG_DEVSTATUS_NULL;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	if (index < 2) 
		return 0;
	return deviceAuthIsSGPayloadTypeValid(table->routing_table[index]->auth, dt);
}



/**
 * \brief Set Router Authentication to strict
 *
 * The Sansgrid Router can make sure a packet received
 * from a device is expected; that is, if the device is still 
 * authenticating, a Chirp would violate the authentication path. \n
 * Strict Authentication means that if an unexpected packet is received,
 * the offending device is dropped from the network. This function enables
 * that functionality.
 * \returns
 * DEV_AUTH_STRICT always
 */
int32_t routingTableRequireStrictAuth(RoutingTable *table) {
	// require strict adherence to routing protocol
	tableAssertValid(table);
	for (int i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			deviceAuthEnable(table->routing_table[i]->auth);
		}
	}
	table->default_strictness = DEV_AUTH_STRICT;
	return DEV_AUTH_STRICT;
}



/**
 * \brief Set Router Authentication to filtered
 *
 * The Sansgrid Router can make sure a packet received
 * from a device is expected; that is, if the device is still 
 * authenticating, a Chirp would violate the authentication path. \n
 * Filtered Authentication means that if an unexpected packet is received,
 * the unexpected packet is dropped, while the offending device remains on the network.
 * This function enables that functionality.
 * \returns
 * DEV_AUTH_FILTERED always
 */
int32_t routingTableSetAuthFiltered(RoutingTable *table) {
	// If we get an unexpected packet, drop it
	// don't drop the offending device though
	tableAssertValid(table);
	for (int i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			deviceAuthEnableFiltered(table->routing_table[i]->auth);
		}
	}
	table->default_strictness = DEV_AUTH_FILTERED;
	return 0;
}



/**
 * \brief Set Router Authentication to loose
 *
 * The Sansgrid Router can make sure a packet received
 * from a device is expected; that is, if the device is still 
 * authenticating, a Chirp would violate the authentication path. \n
 * Loose Authentication turns most of that functionality off. \n
 * The only requirement is that the device eyeballs first.
 * \returns
 * DEV_AUTH_LOOSE always
 */
int32_t routingTableAllowLooseAuth(RoutingTable *table) {
	// allow loose adherence to routing protocol
	tableAssertValid(table);
	for (int i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			deviceAuthEnableLoosely(table->routing_table[i]->auth);
		}
	}
	table->default_strictness = DEV_AUTH_LOOSE;
	return 0;
}



/**
 * \brief Returns the Routing Table's strictness level
 */
int32_t routingTableIsAuthStrict(RoutingTable *table) {
	// check to see if authentication is strict
	tableAssertValid(table);

	return table->default_strictness;
}


/**
 * \brief Lookup the next expected generic packet from a device
 *
 * Returns what general packet we're expecting to get next from a device.
 */
enum SansgridDeviceStatusEnum routingTableLookupNextExpectedPacket(
		RoutingTable *table,
		uint8_t ip_addr[IP_SIZE]) {
	// Check to see what packet we're expecting next from this IP address

	tableAssertValid(table);

	if (!table->table_alloc)
		return SG_DEVSTATUS_NULL;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return SG_DEVSTATUS_NULL;
	return deviceAuthGetNextGeneralPayload(table->routing_table[index]->auth);
}


/**
 * \brief Set the next expected packet for device with IP address ip_addr
 *
 * \param	ip_addr		The device's IP address
 * \param	nextstatus	The Sansgrid Datatype we're expecting next
 * \returns
 * 0 on success \n
 * -1 on error
 */
int32_t routingTableSetNextExpectedPacket(
		RoutingTable *table,
		uint8_t ip_addr[IP_SIZE],
		enum SansgridDeviceStatusEnum nextstatus) {
	// Set what packet we're expecting next from this IP address

	tableAssertValid(table);

	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	return deviceAuthSetNextGeneralPayload(table->routing_table[index]->auth,
			nextstatus);
}


/**
 * \brief Set what general packet was just seen from IP address ip_addr
 * \param		ip_addr			The device's IP address
 * \param		thisstatus		The General datatype of the packet we just saw
 * \returns
 * -1 on error \n
 *  0 on success
 */
int32_t routingTableSetCurrentPacket(
		RoutingTable *table,
		uint8_t ip_addr[IP_SIZE],
		enum SansgridDeviceStatusEnum thisstatus) {
	// Set what packet we just got from this IP address
	
	tableAssertValid(table);

	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	return deviceAuthSetCurrentGeneralPayload(table->routing_table[index]->auth,
			thisstatus);
}



/**
 * \brief Return the general packet datatype that was just seen from device
 * \param		ip_addr		the IP address of the device
 * \returns
 * -1 on error \n
 *  The general packet datatype on success
 */
uint32_t routingTableGetCurrentPacket(
		RoutingTable *table,
		uint8_t ip_addr[IP_SIZE]) {
	// Get what general payload type we're on
	
	tableAssertValid(table);

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	return deviceAuthGetCurrentGeneralPayload(table->routing_table[index]->auth);
}



/**
 * \brief Decrement the Heartbeat Counter and return device status change
 *
 * The routing table keeps a counter that tracks how long ago we heard a device.
 * This function decrements that counter.
 * \returns
 * If a device has just gone stale, or if a device has just been lost, the device's status
 * has just changed. Then return will be a nonzero value. \n
 * Otherwise 0.
 */
int32_t routingTableHeartbeatDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Device is being heartbeat'd
	// This function doesn't actually handle packets, only the heartbeat status
	
	tableAssertValid(table);
	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;

	return hbDecrement(table->routing_table[index]->hb);
}


/**
 * \brief Reset the Heartbeat Counter to full
 * \returns
 * If the device status has changed (ie if the device was stale or lost),
 * then return will be a nonzero value. \n
 * Otherwise return 0
 */
int32_t routingTableHeardDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Heard from device
	// Refresh device health
	tableAssertValid(table);
	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	return hbRefresh(table->routing_table[index]->hb);
}
	


/**
 * \brief Check whether device has been lost
 *
 * \returns
 * 1 if the heartbeat counter is at 0
 * 0 if the heartbeat counter != 0
 */
int32_t routingTableIsDeviceLost(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Check to see if device at ip address has been lost
	
	tableAssertValid(table);
	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	return hbIsDeviceLost(table->routing_table[index]->hb);
}



/**
 * \brief Check whether device is stale
 *
 * \returns
 * 1 if the heartbeat counter is below the stale threshold
 * 0 if the heartbeat counter is above the stale threshold
 */
int32_t routingTableIsDeviceStale(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// Check to see if device at ip address is stale
	
	tableAssertValid(table);
	if (!table->table_alloc)
		return -1;

	uint32_t index = locationToTablePtr(ip_addr, table->base);
	if (index >= ROUTING_ARRAYSIZE || table->routing_table[index] == NULL)
		return -1;
	return hbIsDeviceStale(table->routing_table[index]->hb);
}



/**
 * \brief Return the number of devices allocated in the table
 */
int32_t routingTableGetDeviceCount(RoutingTable *table) {
	// Return the number of devices that are authenticated
	tableAssertValid(table);
	return table->table_alloc;
}



/**
 * \brief Return the status of the routing table
 * \param[in]	devnum		Device to get status on
 * \param[out]	str			formatted information of device status
 */
int32_t routingTableGetStatus(RoutingTable *table, int devnum, char *str) {
	// Print table status
	uint32_t i, j;
	uint8_t ip_addr[IP_SIZE];
	char ip_str[50] = "\0";
	int index = 0;
	int last_was_zero = 0;
	str[0] = '\0';
	uint32_t rdid;
	char payload_name[50];
	syslog(LOG_DEBUG, "Getting Routing Table Info for device %i", devnum);
	syslog(LOG_DEBUG, "table alloc = %i", table->table_alloc);
	for (i=0; i<ROUTING_ARRAYSIZE; i++) {
		if (table->routing_table[i]) {
			if (index == devnum) {
				syslog(LOG_DEBUG, "found one!");
				maskip(ip_addr, table->base, i);
				rdid = routingTableIPToRDID(table, ip_addr);
				sprintf(str, "%4i    ", rdid);
				for (j=0; j<IP_SIZE; j++) {
					if (ip_addr[j] != 0x0) {
						sprintf(ip_str, "%s%.2x", ip_str, ip_addr[j]);
						if (j+1 < IP_SIZE)
							strcat(ip_str, ":");
						last_was_zero = 0;
					} else if (!last_was_zero) {
						strcat(ip_str, ":");
						if (j == 0)
							strcat(ip_str, ":");
						last_was_zero = 1;
					}
				}
				sprintf(str, "%s%35s", str, ip_str);
				if (i == 1) {
					// router
					strcat(str, "\trouter");
				} else if (routingTableIsDeviceLost(table, ip_addr)) {
					// device has been lost
					strcat(str, "\tlost");
				} else if (routingTableIsDeviceStale(table, ip_addr)) {
					// device is stale
					strcat(str, "\tstale");
				} else {
					// device is active
					strcat(str, "\tactive");
				}
				sgPayloadGetPayloadName(
						routingTableGetCurrentPacket(table, ip_addr),
						payload_name);
				if (i == 1) {
					// router
				} else {
					sprintf(str, "%s\t%s", str, payload_name);
				}

				sprintf(str, "%s\n", str);
				break;
			} else {
				index++;
			}
		}
	}
	return 0;
}


/**
 * \brief Increment the internal loop pointer
 *
 * To easily look at all devices in the table, use this function 
 * to iterate through each device.
 * \param[out]		ip_addr		The next IP address
 * \returns
 * 1 if ip_addr has a valid device
 * 0 if we've reached the end of the list
 */
int routingTableStepNextDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	tableAssertValid(table);

	int feach_index = locationToTablePtr(ip_addr, table->base);
	while (++feach_index < ROUTING_ARRAYSIZE) {
		if (feach_index < 2)
			continue;
		if (table->routing_table[feach_index]) {
			maskip(ip_addr, table->base, feach_index);
			table->feach_index = feach_index;
			return 1;
		}
	}
	return 0;
}



/**
 * \brief Initialize the internal loop pointer
 *
 * To easily look at all devices in the table, use this function
 * to start at the beginning of the table, and 
 * [routingTableStepNextDevice](@ref routingTableStepNextDevice) to
 * increment through devices in the table.
 * \param[out]		ip_addr		the IP address of the first device in the table
 * \returns
 * 1 if ip_addr has a valid IP address
 * 0 if there are no devices in the table
 */
int routingTableForEachDevice(RoutingTable *table, uint8_t ip_addr[IP_SIZE]) {
	// setup a loop through all the devices in the table
	tableAssertValid(table);

	table->feach_index = 1;
	maskip(ip_addr, table->base, table->feach_index);
	return routingTableStepNextDevice(table, ip_addr);
}
	

// vim: ft=c ts=4 noet sw=4:
