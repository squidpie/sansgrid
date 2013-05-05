#include "sgRadio.h"


/***********************
*  SansgridRadio Class *
***********************/

SansgridRadio::SansgridRadio(){
	sn_table = NULL;
	payload = NULL;
	ip = NULL;
	Radio = NULL;
}

SansgridRadio::~SansgridRadio(){
	sn_table = NULL;
	payload = NULL;
	ip = NULL;
	Radio = NULL;
}

bool SansgridRadio::mode(enum RadioMode mode) {
	return (router_mode == mode);
}

bool SansgridRadio::setDestAddr(uint64_t addr) {
	uint8_t * at_response = NULL;
	char cmd[64];
	
	//set upper destination address
	sprintf(cmd,"ATDH %d", (addr >> 32)); 
	atCmd(at_response, (const char *)cmd);
	if (at_response == NULL) {
		return 0;
	}
	else if(!IS_OK(at_response)) {
		delete at_response;
		return 0;
	}
	else delete at_response;
	
	// set lower destination address
	sprintf(cmd,"ATDH %d", (addr & 0x0000FFFF)); 
	atCmd(at_response, (const char *)cmd);
	if (at_response == NULL) {
		return 0;
	}
	else if(!IS_OK(at_response)) {
		delete at_response;
		return 0;
	}
	else  delete at_response;
	
	return 1;
}



void SansgridRadio::processSpi() {
	SansgridDataTypeEnum type;
 
	if (payload == NULL) {
		Radio->println("Payload is invalid");
		delay(50);
	}

	memcpy(&type,payload,1);
	
	switch (type) {
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
			Radio->println("HeartBeat Received");
			return;
		case SG_EYEBALL:
			if(MODE(SENSOR)) {
				// Set Destination to Broadcast
				setDestAddr(BROADCAST);
			}
			break;
			
		case SG_FLY:
			if(MODE(ROUTER)) {
				// Set Destination to Broadcast
				setDestAddr(BROADCAST);			}
			break;
			
		case SG_PECK:
			if(MODE(ROUTER)) {
				// Set Destination to Broadcast
				setDestAddr(BROADCAST);
				sn_table->snipInsertIp(&payload[PECK_A_IP], &payload[PECK_SN]);		// store IP at device key for XBsn
			}
			break;
	
		case SG_HATCH: 
			// router ip received
			if (MODE(SENSOR)) 
				router_mode = ROUTER;
				Radio->println("Hello Router");
				delay(50);
			break;
		
		case  SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			if (MODE(ROUTER)) {
				setDestAddr(sn_table->snipSnFromIp(ip));
				//sn_table->snipRemove(ip);
			}
			break;
		
		case SG_CHIRP_SENSOR_DISCONNECT:
			// clean up and prepare to leave network
			break;
		
		default:
			if (MODE(ROUTER)) 
				setDestAddr(sn_table->snipSnFromIp(ip));
			break;
	}
	
	Radio->println("Copying back to packet_out_[f0/f1]");
	delay(50);
	memcpy((packet_out_f0+PACKET_HEADER_SZ),payload,F0_PYLD_SZ);
	Radio->write(packet_out_f0,sizeof(packet_out_f0));
	delay(50);
	memcpy((packet_out_f1+PACKET_HEADER_SZ),(payload+F0_PYLD_SZ),F1_PYLD_SZ);
	Radio->write(packet_out_f1,sizeof(packet_out_f1));
	delay(50);
}

bool SansgridRadio::defrag() {
	bool rv = false;
	if (incoming_packet[PKT_FRAME] == 0) {
		frag_buffer[next][FRAG_PENDING] = 1;
		memcpy(&frag_buffer[next][FRAG_SN],&incoming_packet[PKT_XBSN],XB_SN_LN);
		memcpy(&frag_buffer[next][FRAG_F0],&incoming_packet[PKT_PYLD],F0_PYLD_SZ);
		if ((++next) == FRAG_BUF_SZ) next = 0;
	}
	else {
		for (int i = 0; i < FRAG_BUF_SZ; i++) {
			if (frag_buffer[i][FRAG_PENDING] && memcmp(&frag_buffer[i][FRAG_SN],&incoming_packet[PKT_XBSN],XB_SN_LN)){
				frag_buffer[i][FRAG_PENDING] = 0;
				memcpy(&frag_buffer[i][FRAG_F1], &incoming_packet[PKT_PYLD],F1_PYLD_SZ);
				memcpy(&packet_buffer, &frag_buffer[i][FRAG_F0],sizeof(packet_buffer));
				memcpy(&origin_xbsn, &frag_buffer[i][FRAG_SN], XB_SN_LN);
				rv = true;
				break;
			}
		}
	}
	return rv;
}

void SansgridRadio::processPacket() {
  int index;
	char * tmp;
	uint8_t key[XB_SN_LN];
	
	//Radio->println("process Packet");
	
	SansgridDataTypeEnum type;
	memcpy(&type,packet_buffer,1);
	
	switch (type) {
		case SG_EYEBALL:
			if(MODE(ROUTER)) {
				// insert assigned ip at matched key  entry
		sn_table->snipInsertSn(origin_xbsn, packet_buffer[EYEBALL_SN]);
			}
			break;
	
		case SG_PECK:
			if(MODE(SENSOR)) {
				// Set Destination to Broadcast
				setDestAddr(*((uint64_t *)origin_xbsn));
				sn_table->snipInsertSn(origin_xbsn, &packet_buffer[PECK_SN]);
				sn_table->snipInsertIp(&payload[PECK_R_IP], &payload[PECK_SN]);
			}
			break;

		case  SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			break;
		
		case SG_CHIRP_SENSOR_DISCONNECT:
			// clean up and prepare to leave network
			if (MODE(ROUTER)) {
				//sn_table->snipRemove(snipIpFromSn(origin_xbsn));
			}
			break;
		
		default:
			break;
	}
	SpiData->control = SG_SERIAL_CTRL_VALID_DATA;
	//memcpy(&SpiData->ip_addr, &ip_lookup, IP_SIZE);
	memcpy(SpiData->payload, &packet_buffer, sizeof(SpiData->payload));
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

void SansgridRadio::init(HardwareSerial * xbee_link, SansgridSerial * serial_link, SnIpTable * table_link) {
	Radio = xbee_link;
	sn_table = table_link;
	SpiData = serial_link;
	
	payload = serial_link->payload;
	ip = serial_link->ip_addr;
	setXBsn();
	memset(xbsn,0xAA,XB_SN_LN);
	//memset(&packet_out_f0[0],0,MAX_XB_PYLD);
	//memset(&packet_out_f1[0],0,MAX_XB_PYLD);
	packet_out_f0[PKT_FRAME] = 0x0;
	packet_out_f1[PKT_FRAME] = 0x1;
	memcpy(packet_out_f0,&xbsn,XB_SN_LN);
	memcpy(packet_out_f1,&xbsn,XB_SN_LN);
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
// Radio->println("\nSansgrid is Alive!\n");
	atCmd(cmdOut,"ATSH");
	memcpy((xbsn_str+2),cmdOut,8);
	atCmd(cmdOut,"ATSL");
	memcpy((xbsn_str+8),cmdOut,8);
	//Radio->write(xbsn_str,16);
	atox(xbsn, (char *)xbsn_str, XB_SN_LN); 
	Radio->write(xbsn,XB_SN_LN);
	Radio->println(" xbsn set compelte");
	while(Radio->available() > 0) { Radio->read(); }
	delete xbsn_str;
	delete cmdOut;
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
	//Radio->println("\nEntering Command Mode\n");
	//delay(100);
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
	
	//Radio->println("Exiting Command Mode");
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
