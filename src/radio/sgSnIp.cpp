#include "sgSnIp.h"

/******************
* SnIpTable Class *
******************/

SnIpTable::SnIpTable() {
	size = DEFAULTSNIPSIZE;
	table = new SnIpEntry[DEFAULTSNIPSIZE];
	memset(table,0,size*sizeof(SnIpEntry));
	next = 0;
}

SnIpTable::~SnIpTable() {
	delete table;
}

void SnIpTable::snipExpand() {
	int new_size = size * SNIPEXPANDFACTOR;
	SnIpEntry * expanded = new SnIpEntry[new_size];
	memcpy(table,expanded,(size*sizeof(SnIpEntry)));
	delete table;
	table = expanded;
}


int SnIpTable::snipFindSn(uint8_t * sn){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(&table[index].sn, sn, SNIPBYTEWIDTH)) break;
	}
	return index;
}

int SnIpTable::snipFindIp(uint8_t * ip){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(&table[index].ip, ip, SNIPBYTEWIDTH)) break;
	}
	return index;
}

int SnIpTable::snipFindKey(uint8_t * key){
	int index;
	uint8_t test[SNIPBYTEWIDTH];
	for (index = 0; index < size; index++) {
		for (int i = 0; i < sizeof(key); i++) {
			test[i] = key[i] ^ table[index].sn[i];
		}
		if (!memcmp(test, table[index].ip, SNIPBYTEWIDTH)) break; 
	}
	return index;
}


uint64_t SnIpTable::snipSnFromIp(uint8_t * ip) {
	uint64_t found_sn;
	int index = snipFindIp(ip);
	memcpy(&found_sn, &table[index].sn, SNIPBYTEWIDTH);
	return found_sn;
}

uint64_t SnIpTable::snipIpFromSn(uint8_t * sn) {
	uint64_t found_ip;
	int index = snipFindSn(sn);
	memcpy(&found_ip, &table[index].ip, SNIPBYTEWIDTH);
	return found_ip;
}


void SnIpTable::snipInsertIp(uint8_t * ip, uint8_t * key){
	snipInsert(ip, snipFindKey(key), IP);
}

void SnIpTable::snipInsertIp(uint8_t * ip, int index){
	snipInsert(ip, index, IP);
}

void SnIpTable::snipInsertSn(uint8_t * sn, uint8_t * key){
	snipInsert(sn, next, SN);
	for (int i = 0; i < sizeof(key); i++) {
		key[i] ^= sn[i];
	}
	snipInsert(key, next, IP);
}

void SnIpTable::snipInsertSn(uint8_t * sn, int index){
	snipInsert(sn, index, SN);
}

void SnIpTable::snipInsert(uint8_t * data, int index, SnTableIndex type) {
	uint8_t * entry;
	switch(type) {
		case IP:
			entry = &table[index].ip[0];
			break;
		case SN:
			entry = &table[index].sn[0];
			break;
	}
	memcpy(data, entry, SNIPBYTEWIDTH);
	if (index == next) {
		size++;
		next++;
	}
}
// end of SnIpTable Class
