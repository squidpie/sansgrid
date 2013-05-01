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

SansgridRadio::SansgridRadio(HardwareSerial * xbee_link, SansgridSerial * serial_link, SnIpTable * table_link){
	Radio = xbee_link;
	sn_table = table_link;
	payload = serial_link->payload;
	ip = serial_link->ip_addr;
}

SansgridRadio::~SansgridRadio(){
	sn_table = NULL;
	payload = NULL;
	ip = NULL;
	Radio = NULL;
}

void SansgridRadio::processSpi() {
	SansgridDataTypeEnum type;
	memcpy(&type,payload,1);
	switch (type) {
		case SG_EYEBALL:
			
		case SG_FLY:
		case SG_PECK:
			// set broadcast flag
			break;
		case SG_HATCH: 
			// router ip received
			break;
		case  SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
		case SG_CHIRP_SENSOR_DISCONNECT:
			// clean up and prepare to leave network
			break;
		default:
			break;
	}
	memcpy((packet_out_f0+9),payload,50);
	memcpy((packet_out_f1+9),(payload+50),31);
}

void SansgridRadio::loadFrame(int frame) {
	uint8_t * p = NULL;
	switch (frame) {
		case 0:
			p = packet_out_f0;
			break;
		case 1:
			p = packet_out_f1;
			break;
		}
	
	if (p != NULL) {
		memcpy(packet_buffer,p,sizeof(packet_buffer));
	}

}

void SansgridRadio::init() {
	setXBsn();
	memcpy(packet_out_f0,&xbsn,XB_SN_LN);
	memcpy(packet_out_f1,&xbsn,XB_SN_LN);
	packet_out_f0[XB_SN_LN] = 0x0;
	packet_out_f1[XB_SN_LN] = 0x1;
	return;
}

bool SansgridRadio::rxComplete() {
	return true;
}

void SansgridRadio::setXBsn() {
	uint8_t * xbsn_str = new uint8_t[16];
	uint8_t * cmdOut = new uint8_t[8];
	memset(xbsn_str,0,16);
	memset(cmdOut,0,8);
	Radio->println("\nSansgrid is Alive!\n");
	atCmd(cmdOut,"ATSH");
	memcpy((xbsn_str+2),cmdOut,8);
	atCmd(cmdOut,"ATSL");
	memcpy((xbsn_str+8),cmdOut,8);
	Radio->write(xbsn_str,16);
	atox(xbsn, (char *)xbsn_str, 8); 
	while(Radio->available() > 0) { Radio->read(); }
	delete xbsn_str;
}

void SansgridRadio::read() {
	//Radio->println("Radio is reading");
	int i = 0;
  while(Radio->available() > 0 && i < MAX_XB_PYLD) {
    delay(2);
    packet_buffer[i++] = Radio->read();
  }
	//processPacket();
}

void SansgridRadio::set_mode(RadioMode mode) {
	router_mode = mode;
}

void SansgridRadio::write() {
	//processPacket();
  Radio->write(packet_buffer,sizeof(packet_buffer));
}

void SansgridRadio::atCmd(uint8_t * result,const char * cmd) {
	int i = 0;
	Radio->println("\nEntering Command Mode\n");
	delay(100);
	while (Radio->available() > 0) { Radio->read();}
	while(Radio->available() == 0) {
		if (i > 3) {
			Radio->println("ATCN");
			delay(1200);
			Radio->println("\nCommand Failed on +++ timeout\n");
			return;
		}
		for (int z = 0; z < 3; z++) {
			Radio->print("+");
			delay(50);
		}
		delay(3000);
		i++;
	}
	Radio->println();
	i = 0;
	
	while(Radio->available() > 0) {
		Radio->read();
	}
	
	//Radio->println();

//if (strncmp(result, "OK", sizeof("OK"))) {
		Radio->println(cmd);

		delay(1200);
		uint8_t buffer[8];
		i = 0;
		while(Radio->available() > 0 && i < 8 ){//PACKET_SZ) {
//			memset((result+i),Radio->read(),1);
		  buffer[i] = Radio->read();
		//	Radio->write
			i++;
			delay(2);
		}	
		Radio->println();
		delay(100);
		Radio->println("ATCN");
		delay(1000);
	//debugger->debug(NOTIFICATION,__FUNC__,tmp);
//	}
//	else {
	//	Radio->println("ATCN");
	//}
	
	Radio->println("Exiting Command Mode");
	memcpy(result,&buffer[0],8);
}


uint8_t * SansgridRadio::genDevKey(uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn) {
	uint8_t * key;
	key = new uint8_t[SNIPBYTEWIDTH];
	for (int i = 0; i < SNIPBYTEWIDTH/2; i+=2) {
		key[i] = man_id[i];
		key[i+1] = mod_id[i];
	}
	for (int i = 0; i < SNIPBYTEWIDTH; i++) {
		key[i] ^= dev_sn[i];
	}
	return key;
}

void SansgridRadio::processPacket() {
  int index;
	char * tmp;
	uint8_t * key;
	//Radio->println("process Packet");
  if (router_mode) {
			//debugger->debug(NOTIFICATION,__FUNC__,"Router Mode");
      if(packet_buffer[PACKET_ID] == SG_EYEBALL) {
				//Radio->println("Eyeball Received");
  			//debugger->debug(NOTIFICATION,__FUNC__,"Eyeball Packet");
				sn_table->snIpInsertSn(&packet_buffer[XBSN]);
				
				Radio->write(packet_buffer,PACKET_SZ);
				delay(50);	
      }
      if (packet_buffer[PACKET_ID] == SG_PECK) {
				Radio->println("Peck Received");
				//debugger->debug(NOTIFICATION,__FUNC__,"Peck Packet");
        int sn;
        sn = btoi(&packet_buffer[PECKSN], SN_LENGTH);
				sprintf(tmp,"%X",packet_buffer[PECKSN]);
        //debugger->debug(NOTIFICATION,__FUNC__,tmp);
				key = genDevKey(&packet_buffer[PECKMANID],&packet_buffer[PECKMODID],&packet_buffer[PECKSN]);
        sn_table->snIpInsertIp(&packet_buffer[PECKIP],key);
			//	index = findSn(sn);
				//assert(index < IP_TABLE_SZ);
        //memcpy(&packet_buffer[PECKIP],&sn_table[index][IP],IP_LENGTH);
      }
  }
  else {
    //debugger->debug(NOTIFICATION,__FUNC__,"Sensor Mode"); 
  	char tmp[1024];
		//atCmd(&tmp[0],"ATID");
	}
 
}

/**************************
* Global Helper Functions * 
**************************/
/*
void sgDebugInit(SerialDebug * db) {
		//debugger = db;
		//debugger->debug(NOTIFICATION,__FUNC__,"Serial Debugger for Radio Setup");
		delay(50);
}
*/
int btoi(byte * b,int ln) {
  int rv = 0; 
  int s_i = 0;
  for (int i = ln - 1 ; i >= 0; i--){
     rv += *(b+i) << s_i;
     s_i += 8;
  }
  return rv;
}

void atox(uint8_t *hexarray, char *str, uint32_t hexsize) {
	// convert the full string of hex values into an array
	uint32_t i_str, i_hex = 0;
	int increment = 0;
	char chunk[3];
	uint32_t length;
	uint32_t uval;

	memset(hexarray,0,hexsize);
	if (str == NULL)
		return;
	length = strlen(str);
	//offset = hexsize - ((length+1)/2);

	for (i_str=0; i_str<length;) {
		if (((length ^ i_str) & 0x1) == 0x1) {
			// make sure we catch case of 0x123
			// where we should parse as 0x1  0x23
			chunk[0] = str[i_str];
			chunk[1] = '\0';
			//sscanf(&str[i], "%1s", chunk);
			increment = 1;
		} else {
			chunk[0] = str[i_str];
			chunk[1] = str[i_str+1];
			chunk[2] = '\0';
			//sscanf(&str[i], "%2s", chunk);
			increment = 2;
		}
		sscanf(chunk, "%x", &uval);
		hexarray[i_hex++] = (uval & 0xff);
		i_str+=increment;
	}
	return;
}

// Stubs
void write_spi() {
  // stub
}

void read_spi() {
  // stub
}
