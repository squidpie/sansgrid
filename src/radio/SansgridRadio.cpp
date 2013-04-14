#include "SansgridRadio.h"

SnIpTable_t *snIpExpand(SnIpTable_t * current) {
	SnIpTable_t * expanded = new SnIpTable_t[SNIPEXPANDFACTOR*current->size];
	memcpy(current,expanded,current->size*sizeof(*current));
	return expanded;
}


int snIpfindSn(SnIpTable_t * table, uint64_t sn){
	int index;
	for (index = 0; index < table->count; index++) {
		if (table->head[index][SN] == sn) break;
	}
	return index;
}

int snIpfindIp(SnIpTable_t * table, uint64_t ip){
	int index;
	for (index = 0; index < table->count; index++) {
		if (table->head[index][IP] == sn) break;
	}
	return index;
}

int snIpGetEmptyIndex(SnIpTable_t * table){

}

void snIpInsertIp(uint64_t ip, int index){

}

void snIpInsertSn(uint64_t sn, int index){

}

void sgDebugInit(SerialDebug &db) {
	debugger = db;
	debugger.debug(NOTIFICATION,__FUNC__,"debug init complete\n");
	delay(100);
}

SansgridRadio::SansgridRadio(SansgridSerial * data) {
	Radio = &Serial;
	Radio->begin(9600);
	router_mode = SENSOR;
	packet = &(data->payload[0]);
}

SansgridRadio::~SansgridRadio() {
	Radio->end();
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

int SansgridRadio::findSn(int sn) {
  int rv;
  for (rv = 0; rv < IP_TABLE_SZ; rv++) {
    if (sn_table[rv][IP] == sn && sn_table[rv][SN] == sn) break;
  }
    
  return rv;
}

int SansgridRadio::getEmptySnIndex() {
  int rv;
  for (rv = 0; rv < IP_TABLE_SZ; rv++) {
    if (sn_table[rv][SN] == 0 && sn_table[rv][IP] == 0) break;
  }
  return rv;
}

void SansgridRadio::processPacket() {
  int index;
	char * tmp;
  if (router_mode) {
			debugger.debug(NOTIFICATION,__FUNC__,"Router Mode");
      if(packet_buffer[PACKET_ID] == EYEBALL_TYPE) {
  			debugger.debug(NOTIFICATION,__FUNC__,"Eyeball Packet");
        index = getEmptySnIndex();
     		//assert(index < IP_TABLE_SZ);
				atCmd(tmp,"ATID");
        memcpy(&packet_buffer[EYE_SN_ENTRY],&sn_table[index][SN],SN_LENGTH);
        memcpy(&packet_buffer[EYE_SN_ENTRY],&sn_table[index][IP],SN_LENGTH);
      }
      if (packet_buffer[PACKET_ID] == PECK_TYPE) {
				debugger.debug(NOTIFICATION,__FUNC__,"Peck Packet");
        int sn;
        sn = btoi(&packet_buffer[PECK_SN_ENTRY], SN_LENGTH);
				sprintf(tmp,"%X",packet_buffer[PECK_SN_ENTRY]);
        debugger.debug(NOTIFICATION,__FUNC__,tmp);
        index = findSn(sn);
				//assert(index < IP_TABLE_SZ);
        memcpy(&packet_buffer[PECK_IP_ENTRY],&sn_table[index][IP],IP_LENGTH);
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
