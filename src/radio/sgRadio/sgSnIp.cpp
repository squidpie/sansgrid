/* sansgridRadio Route over XBee Routing Table
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

#include "sgSnIp.h"

/******************
* SnIpTable Class *
******************/

SnIpTable::SnIpTable() {
	size = DEFAULTSNIPSIZE;
	//table = new SnIpEntry[DEFAULTSNIPSIZE];
	memset(table,0,size*sizeof(SnIpEntry));
	next = 0;
}

SnIpTable::~SnIpTable() {
	delete table;
}

void SnIpTable::snipRemove(uint8_t * sn) {
	int index = snipFindSn(sn);
	if (index < 0) return;
	memset(table[index].sn, 0, SNIP_XB_WIDTH);
	memset(table[index].ip, 0, SNIP_IP_WIDTH);
	memset(table[index].dev_sn, 0, SNIP_XB_WIDTH);
}

void SnIpTable::snipExpand() {
	/*int new_size = size * SNIPEXPANDFACTOR;
	SnIpEntry * expanded = new SnIpEntry[new_size];
	memcpy(table,expanded,(size*sizeof(SnIpEntry)));
	delete table;
	table = expanded;*/
}

int SnIpTable::snipFindSn(uint8_t * sn){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(table[index].sn, sn, SNIP_XB_WIDTH)) {
			return index;
		}
	}
	return -1;
}

int SnIpTable::snipFindDevSn(uint8_t * dev) {
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(table[index].dev_sn, dev, SNIP_XB_WIDTH)) {
			return index;
		}
	}
	//if (index < 0 ) return 0;
	return -1;
}
	

int SnIpTable::snipFindIp(uint8_t * ip){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(table[index].ip, ip, SNIP_IP_WIDTH)) { 
			return index;
		}
	}
	return -1;
}

int SnIpTable::snipFindKey(uint8_t * key){
	int index;
	uint8_t test[SNIP_IP_WIDTH];
	memset(test, 0, SNIP_IP_WIDTH);
	for (index = 0; index < size; index++) {
		for (int i = 0; i < SNIP_XB_WIDTH; i++) {
			test[i] = key[i];// ^ table[index].sn[i];
		}
		if (!memcmp(test, table[index].ip, SNIP_IP_WIDTH)) {
			return index;
		}
	}
	return -1;
}


void SnIpTable::snipSnFromIp(uint8_t * sn, uint8_t * ip) {
	int index = snipFindIp(ip);
	if (index < 0) { 
		memset(sn, 0xFE, SNIP_XB_WIDTH);
		return;
	}
	memcpy(sn, table[index].sn, SNIP_XB_WIDTH);
}

void SnIpTable::snipIpFromSn(uint8_t * ip, uint8_t * sn) {
	int index = snipFindSn(sn);
	if (index < 0) { 
		memset(ip, 0xFE, SNIP_IP_WIDTH);
		return;
	}
	memcpy(ip, table[index].ip, SNIP_IP_WIDTH);
}



int SnIpTable::snipInsertIp(uint8_t * ip, uint8_t * dev){
	int index = snipFindDevSn(dev);
	if (index < 0) {
		return -1;// insert hack here for single sensor network if this piece of shit fails.
	}
	snipInsert(ip, index, IP);
	return index;
	//snipIndert(ip, snipFindSn(key), IP);
}

/*
void SnIpTable::snipInsertIp(uint8_t * ip, int index){
	snipInsert(ip, index, IP);
}
*/

int SnIpTable::snipGetIndex() {
	uint8_t empty[SNIP_XB_WIDTH];
	int index = 0;
	memset(empty, 0, SNIP_XB_WIDTH);
	for (index = 0; index < size; index++) {
		if (!memcmp(empty, table[index].sn, SNIP_XB_WIDTH)) {
			return index;
		}
	}
	return -1;
}

int SnIpTable::snipInsertSn(uint8_t * sn, uint8_t * dev){
	int insert_pos = snipFindDevSn(dev);
	if (insert_pos < 0) {
		insert_pos = snipGetIndex();
		if (insert_pos < 0) return -1;
	}
	else {
		memset(table[insert_pos].sn, 0, SNIP_XB_WIDTH);
		memset(table[insert_pos].ip, 0, SNIP_IP_WIDTH);
		memset(table[insert_pos].dev_sn, 0, SNIP_XB_WIDTH);
	}
	snipInsert(sn, insert_pos, SN);
	snipInsert(dev, insert_pos, DEV);
	return insert_pos;
}

/*
void SnIpTable::snipInsertSn(uint8_t * sn, int index){
	snipInsert(sn, index, SN);
}
*/

void SnIpTable::snipInsert(uint8_t * data, int index, SnTableIndex type) {
	uint8_t * entry;
	int width;
	switch(type) {
		case IP:
			entry = table[index].ip;
			width = SNIP_IP_WIDTH;
			break;
		case SN:
			entry = table[index].sn;
			width = SNIP_XB_WIDTH;
			break;
		case DEV:
			entry = table[index].dev_sn;
			width = SNIP_XB_WIDTH;
			break;
		default:
			entry = NULL;
	}
	if (entry == NULL || index < 0) {
		return;
	}
	
	for (int i = 0; i < width; i++) {
		entry[i] = data[i];
	}
	
	if (index == next) {
		next++; 
	}
	if (next == size) {
		next = 0;//snipExpand();
	}
}

void SnIpTable::snipDebug(HardwareSerial * debug) {
	for (int i = 0; i < size; i++) {
		debug->print("SN: ");
		debug->write(table[i].sn, SNIP_XB_WIDTH);
		debug->println();
		debug->print("IP: ");
		debug->write(table[i].ip, SNIP_IP_WIDTH);
		debug->println();
		debug->print("DEV SN: ");
		debug->write(table[i].dev_sn, SNIP_XB_WIDTH);
		debug->println();
	}
}
// end of SnIpTable Class
