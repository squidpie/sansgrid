#include "sgRadio.h"

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

void SnIpTable::snIpExpand() {
	int new_size = size * SNIPEXPANDFACTOR;
	SnIpEntry * expanded = new SnIpEntry[new_size];
	memcpy(table,expanded,(size*sizeof(SnIpEntry)));
	delete table;
	table = expanded;
}


int SnIpTable::snIpFindSn(uint8_t * sn){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(&table[index].sn, sn, SNIPBYTEWIDTH)) break;
	}
	return index;
}

int SnIpTable::snIpFindIp(uint8_t * ip){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(&table[index].ip, ip, SNIPBYTEWIDTH)) break;
	}
	return index;
}

void SnIpTable::snIpInsertIp(uint8_t * ip, uint8_t * key){
	snIpInsert(ip, snIpFindIp(key), IP);
}

void SnIpTable::snIpInsertIp(uint8_t * ip, int index){
	snIpInsert(ip, index, IP);
}

void SnIpTable::snIpInsertSn(uint8_t * sn){
	snIpInsert(sn, next, SN);
}

void SnIpTable::snIpInsertSn(uint8_t * sn, int index){
	snIpInsert(sn, index, SN);
}

void SnIpTable::snIpInsert(uint8_t * data, int index, SnTableIndex type) {
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


/***********************
*  SansgridRadio Class *
***********************/

SansgridRadio::SansgridRadio(SansgridSerial * serial_link, SnIpTable * table_link){
	sn_table = table_link;
	packet = serial_link->payload;
}

SansgridRadio::~SansgridRadio(){
	sn_table = NULL;
	packet = NULL;
}

void SansgridRadio::read() {
  for (int i = 0; Radio->available() > 0 && i < PACKET_SZ; i++) {
    delay(1);
    packet_buffer[i] = Radio->read();
  }
	processPacket();
}

void SansgridRadio::set_mode(RadioMode mode) {
	router_mode = mode;
}

void SansgridRadio::write() {
	processPacket();
  Radio->write(packet_buffer,sizeof(packet_buffer));
}

void SansgridRadio::atCmd(char * result,const char * cmd) {
	int i = 0;
	Radio->println("+++");
	delay(300);
	while(Radio->available() > 0) {
		Radio->read();
	}
	Radio->println(cmd);
	while(Radio->available() > 0 && i < sizeof(result)) {
		result[i++] = Radio->read();
	}	
	Radio->println("ATCN");
}


uint8_t * SansgridRadio::genDevKey(uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn) {
	uint8_t * key;
	key = new uint8_t[SNIPBYTEWIDTH];
	for (int i = 0; i < SNIPBYTEWIDTH/2; i++) {
		key[i] = man_id[i];
		key[++i] = mod_id[i];
	}
	for (int i = 0; i < SNIPBYTEWIDTH; i++) {
		key[i] ^= dev_sn[i];
	}
	return key;
}

void SansgridRadio::processPacket() {
  int index;
	char * tmp;
  if (router_mode) {
			debugger->debug(NOTIFICATION,__FUNC__,"Router Mode");
      if(packet_buffer[PACKET_ID] == SG_EYEBALL) {
  			debugger->debug(NOTIFICATION,__FUNC__,"Eyeball Packet");
				sn_table->snIpInsertSn(&packet_buffer[XBSN]);
      }
      if (packet_buffer[PACKET_ID] == SG_PECK) {
				debugger->debug(NOTIFICATION,__FUNC__,"Peck Packet");
        int sn;
        sn = btoi(&packet_buffer[PECKSN], SN_LENGTH);
				sprintf(tmp,"%X",packet_buffer[PECKSN]);
        debugger->debug(NOTIFICATION,__FUNC__,tmp);
        sn_table->snIpInsertIp(&packet_buffer[PECKIP],genDevKey(&packet_buffer[PECKMANID],&packet_buffer[PECKMODID],&packet_buffer[PECKSN]));
			//	index = findSn(sn);
				//assert(index < IP_TABLE_SZ);
        //memcpy(&packet_buffer[PECKIP],&sn_table[index][IP],IP_LENGTH);
      }
  }
  else {
    debugger->debug(NOTIFICATION,__FUNC__,"Sensor Mode"); 
  }
 
}

/**************************
* Global Helper Functions * 
**************************/

void sgDebugInit(SerialDebug * db) {
		debugger = db;
}

int btoi(byte * b,int ln) {
  int rv = 0; 
  int s_i = 0;
  for (int i = ln - 1 ; i >= 0; i--){
     rv += *(b+i) << s_i;
     s_i += 8;
  }
  return rv;
}

// Stubs
void write_spi() {
  // stub
}

void read_spi() {
  // stub
}
