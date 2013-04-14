#include "sgRadio.h"

SnIpTable::SnIpTable() {
	table = new SnIpEntry[DEFAULTSNIPSIZE][SNIPTABLEWIDTH][SNIPBYTEWIDTH];
	memset(table,0,DEFAULTSNIPSIZE * (SNIPTABLEWIDTH * SNIPBYTEDIWTH));
	size = DEFAULTSNIPSIZE;
	next = 0;
}

SnIpTable::~SnIpTable() {
	delete [] table;
}

void SnIpTable::snIpExpand() {
	SnIpEntry * expanded = new SnIpEntry[SNIPEXPANDFACTOR*size];
	memcpy(table,expanded,(size*sizeof(SnIpEntry)));
	delete [] table;
	table = expanded;
}


int SnIpTable::snIpfindSn(uint8_t * sn){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(&table[index][SN], sn, SNIPBYTEWIDTH)) break;
	}
	return index;
}

int SnIpTable::snIpfindIp(uint8_t * ip){
	int index;
	for (index = 0; index < size; index++) {
		if (!memcmp(&table[index][IP], ip, SNIPBYTEWIDTH)) break;
	}
	return index;
}

void SnIpTable::snIpInsertIp(uint8_t * ip, uint8_t * key){
	snIpInsert(ip, snIpFindIp(key), IP);
}

void SnIpTable::snIpInsertIp(uint8_t * ip, int index){
	snIpInsert(ip, index, IP);
}

void snIpInsertSn(uint8_t * sn, int index){
	snIpInsert(sn, index, SN);
}

void snIpInsert(uint8_t * data, int index, SnTableIndex type) {
	memcpy(data, &table[index][type], SNIPBYTEWIDTH);
	if (index == next) {
		size++;
		next++;
	}
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

int SansgridRadio::getEmptySnIndex() {
  int rv;
  for (rv = 0; rv < IP_TABLE_SZ; rv++) {
    if (sn_table[rv][SN] == 0 && sn_table[rv][IP] == 0) break;
  }
  return rv;
}

uint8_t * genDevKey() {
	uint8_t * key;
	key = new uint8_t[SNIPBYTEWIDTH];
	for (int i = 0; i < SNIPBYTEWIDTH/2; i++) {
		key[i] = packet_buffer[MANID];
		key[++i] = packet_buffer[MODID];
	}
	for (int i = 0; i < SNIPBYTEWIDTH; i++) {
		key[i] ^= packet_buffer[DEVSN];
	}
	return key;
}

void SansgridRadio::processPacket() {
  int index;
	char * tmp;
  if (router_mode) {
			debugger.debug(NOTIFICATION,__FUNC__,"Router Mode");
      if(packet_buffer[PACKET_ID] == EYEBALL_TYPE) {
  			debugger.debug(NOTIFICATION,__FUNC__,"Eyeball Packet");
				sn_table->snIpInsertSn(&packet_buffer[XBSN]);
      }
      if (packet_buffer[PACKET_ID] == PECK_TYPE) {
				debugger.debug(NOTIFICATION,__FUNC__,"Peck Packet");
        int sn;
        sn = btoi(&packet_buffer[PECK_SN_ENTRY], SN_LENGTH);
				sprintf(tmp,"%X",packet_buffer[PECK_SN_ENTRY]);
        debugger.debug(NOTIFICATION,__FUNC__,tmp);
        sn_table->snIpInsertIp(&packet_buffer[PECK_IP_ENTRY],genDevKey());
			//	index = findSn(sn);
				//assert(index < IP_TABLE_SZ);
        //memcpy(&packet_buffer[PECK_IP_ENTRY],&sn_table[index][IP],IP_LENGTH);
      }
  }
  else {
    debugger.debug(NOTIFICATION,__FUNC__,"Sensor Mode"); 
  }
 
}

void write_spi() {
  // stub
}

void read_spi() {
  // stub
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
