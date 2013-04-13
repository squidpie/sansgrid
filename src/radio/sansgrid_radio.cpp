#include "sansgrid_radio.h"

void sansgrid_debug_init(SerialDebug &db) {
	debugger = db;
	debugger.debug(NOTIFICATION,__FUNC__,"debug init complete\n");
	delay(100);
}

sansgrid_radio::sansgrid_radio() {
	radio = &Serial;
	radio->begin(9600);
	router_mode = SENSOR;	
}

sansgrid_radio::~sansgrid_radio() {
	radio->end();
}

void sansgrid_radio::read() {
  for (int i = 0; radio->available() > 0 && i < PACKET_SZ; i++) {
    delay(1);
    packet_buffer[i] = radio->read();
  }
	process_packet();
}

void sansgrid_radio::set_mode(radio_mode mode) {
	router_mode = mode;
}

void sansgrid_radio::write() {
	process_packet();
  radio->write(packet_buffer,sizeof(packet_buffer));
}

void sansgrid_radio::at_cmd(char * result,const char * cmd) {
	int i = 0;
	radio->println("+++");
	delay(300);
	while(radio->available() > 0) {
		radio->read();
	}
	radio->println(cmd);
	while(radio->available() > 0 && i < sizeof(result)) {
		result[i++] = radio->read();
	}	
	radio->println("ATCN");
}

int sansgrid_radio::find_pending_ip(int sn) {
  int rv;
  for (rv = 0; rv < IP_TABLE_SZ; rv++) {
    if (ip_table[rv][IP] == sn && ip_table[rv][SN] == sn) break;
  }
    
  return rv;
}

int sansgrid_radio::next_sn_entry() {
  int rv;
  for (rv = 0; rv < IP_TABLE_SZ; rv++) {
    if (ip_table[rv][SN] == 0 && ip_table[rv][IP] == 0) break;
  }
  return rv;
}

void sansgrid_radio::process_packet() {
  int index;
	char * tmp;
  if (router_mode) {
			debugger.debug(NOTIFICATION,__FUNC__,"Router Mode");
      if(packet_buffer[PACKET_ID] == EYEBALL_TYPE) {
  			debugger.debug(NOTIFICATION,__FUNC__,"Eyeball Packet");
        index = next_sn_entry();
     		//assert(index < IP_TABLE_SZ);
				at_cmd(tmp,"ATID");
        memcpy(&packet_buffer[EYE_SN_ENTRY],&ip_table[index][SN],SN_LENGTH);
        memcpy(&packet_buffer[EYE_SN_ENTRY],&ip_table[index][IP],SN_LENGTH);
      }
      if (packet_buffer[PACKET_ID] == PECK_TYPE) {
				debugger.debug(NOTIFICATION,__FUNC__,"Peck Packet");
        int sn;
        sn = byte2int(&packet_buffer[PECK_SN_ENTRY], SN_LENGTH);
				sprintf(tmp,"%X",packet_buffer[PECK_SN_ENTRY]);
        debugger.debug(NOTIFICATION,__FUNC__,tmp);
        index = find_pending_ip(sn);
				//assert(index < IP_TABLE_SZ);
        memcpy(&packet_buffer[PECK_IP_ENTRY],&ip_table[index][IP],IP_LENGTH);
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
int byte2int(byte * b,int ln) {
  int rv = 0; 
  int s_i = 0;
  for (int i = ln - 1 ; i >= 0; i--){
     rv += *(b+i) << s_i;
     s_i += 8;
  }
  return rv;
}
