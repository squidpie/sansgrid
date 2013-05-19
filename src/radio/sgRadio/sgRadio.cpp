#include "sgRadio.h"


/***********************
*  SansgridRadio Class *
***********************/

SansgridRadio::SansgridRadio(){
/*	sn_table = NULL;
	payload = NULL;
	ip = NULL;
	Radio = NULL;*/
}


bool SansgridRadio::mode(enum RadioMode mode) {
	return (router_mode == mode);
}

bool SansgridRadio::setDestAddr(uint64_t addr) {
	uint8_t at_response[12];
	char cmd[80];
	memset(at_response,'\0',12);
	//set upper destination address
	sprintf(cmd,"ATDH %x", (addr >> 32)); 
	atCmd(at_response, (const char *)cmd);
	if (at_response[0] == '\0') {
		return 0;
	}
	else if(!IS_OK(at_response)) {
		Radio->println("AT cmd Failed with");
		//delete at_response;
		return 0;
	}
	
	// set lower destination address
	memset(at_response,'\0',12);
	sprintf(cmd,"ATDL %x", (addr & 0x00000000FFFFFFFF)); 
	atCmd(at_response, (const char *)cmd);
	if (at_response == '\0') {
		return 0;
	}
	else if(!IS_OK(at_response)) {
		return 0;
	}
	
	return 1;
}



void SansgridRadio::processSpi() {
	SansgridDataTypeEnum type;
	//Radio->println("Process Spi"); 
	if (payload == NULL) {
		Radio->println("Payload is invalid");
		delay(50);
	}

	memcpy(&type,payload,1);
	
	switch (type) {
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
			//Radio->println("HeartBeat Received");
			return;
		case SG_EYEBALL:
			if(MODE(SENSOR)) {
				// Set Destination to Broadcast
				// setDestAddr(0x0);
			}
			break;
			
		case SG_FLY:
			if(MODE(ROUTER)) {
				// Set Destination to Broadcast
				// setDestAddr(BROADCAST);			
			}
			break;
			
		case SG_PECK:
			if(MODE(ROUTER)) {
				// Set Destination to Broadcast
				// setDestAddr(BROADCAST);
				sn_table->snipInsertIp(&payload[PECK_A_IP], &payload[PECK_SN]);		// store IP at device key for XBsn
			}
			break;
	
		case SG_HATCH: 
			// router ip received
			if (MODE(SENSOR)) 
				router_mode = ROUTER;
//				setDestAddr(BROADCAST);
				//Radio->println("Hello Router");
				//delay(50);
			break;
		
		case  SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			if (MODE(ROUTER)) {
				//setDestAddr(sn_table->snipSnFromIp(ip));
				//sn_table->snipRemove(ip);
			}
			break;
		
		case SG_CHIRP_SENSOR_DISCONNECT:
			// clean up and prepare to leave network
			break;
		
		default:
			if (MODE(ROUTER)) 
				//setDestAddr(sn_table->snipSnFromIp(ip));
			break;
	}
	
	//Radio->println("Copying back to packet_out_[f0/f1]");
	//delay(50);
	if (MODE(ROUTER)) {
		uint64_t sn_tmp = sn_table->snipSnFromIp(ip);
		memcpy(origin_xbsn,&sn_tmp,XB_SN_LN); // TODO this might not work
		memcpy(&pkt0_frag[1],origin_xbsn,XB_SN_LN);
		memcpy(&pkt1_frag[1],origin_xbsn,XB_SN_LN);
	}
	memcpy(&pkt0_frag[PACKET_HEADER_SZ],payload,F0_PYLD_SZ);
	//Radio->write(pkt0_frag,sizeof(pkt0_frag));
	//delay(50);
	memcpy(&pkt1_frag[PACKET_HEADER_SZ],(payload+F0_PYLD_SZ),F1_PYLD_SZ);
	//Radio->write(pkt1_frag,MAX_XB_PYLD);
	//delay(50);
}

bool SansgridRadio::defrag() {
	bool rv = false;
	if (MODE(SENSOR)) {
		memcpy(origin_xbsn, (incoming_packet + 1), XB_SN_LN);
		if (memcmp(origin_xbsn,xbsn,XB_SN_LN) != 0) {
			return rv;
		}
	}
	if (incoming_packet[PKT_FRAME] == 0) {
		frag_buffer[next][FRAG_PENDING] = 1;
		memcpy(&frag_buffer[next][FRAG_SN],&incoming_packet[PKT_XBSN],XB_SN_LN);
		memcpy(&frag_buffer[next][FRAG_F0],&incoming_packet[PKT_PYLD],F0_PYLD_SZ);
		//debug print
		//Radio->write(next);
		//uint8_t * addr = &frag_buffer[next][FRAG_SN]; 
		//Radio->write((unsigned int)addr); delay(100);
		//Radio->write(0xFF);
		if ((++next) == FRAG_BUF_SZ) next = 0;
	}
	else {
		for (int i = 0; i < FRAG_BUF_SZ; i++) {
			//Radio->write(i);
			//uint8_t * addr = &frag_buffer[i][FRAG_SN];
		//	Radio->write((unsigned int)addr); delay(100);
			//Radio->write(0xFF);
			if (frag_buffer[i][FRAG_PENDING] && !memcmp(&frag_buffer[i][FRAG_SN],&incoming_packet[PKT_XBSN],XB_SN_LN)){
				//Radio->println("Frame 2 received"); delay(100);
				frag_buffer[i][FRAG_PENDING] = 0;
				memcpy(&frag_buffer[i][FRAG_F1], &incoming_packet[PKT_PYLD],F1_PYLD_SZ);
				memcpy(packet_buffer, &frag_buffer[i][FRAG_F0],SG_PACKET_SZ);
				memcpy(origin_xbsn, &frag_buffer[i][FRAG_SN], XB_SN_LN); // TODO check for correctness
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
				// Set Destination 
				//setDestAddr(*((uint64_t *)origin_xbsn));
				//sn_table->snipInsertSn(origin_xbsn, &packet_buffer[PECK_SN]);
				//sn_table->snipInsertIp(&payload[PECK_R_IP], &payload[PECK_SN]);
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
	memset(SpiData->ip_addr, 0xDD, IP_SIZE);
	//memcpy(&SpiData->ip_addr, &ip_lookup, IP_SIZE);
	memcpy(SpiData->payload, packet_buffer, sizeof(SpiData->payload));
}

void SansgridRadio::loadFrame(int frame) {
	//uint8_t * p = NULL;
	switch (frame) {
		case 0:
			pending_packet = pkt0_frag;
			break;
		case 1:
			pending_packet = pkt1_frag;
			break;
		default:
			pending_packet = NULL;
		}
/*	
	if (p != NULL) {
		memcpy(packet_buffer,p,MAX_XB_PYLD);
	}*/

}

void SansgridRadio::init(HardwareSerial * xbee_link, SansgridSerial * serial_link, SnIpTable * table_link) {
	Radio = xbee_link;
	sn_table = table_link;
	SpiData = serial_link;

	payload = serial_link->payload;
	ip = serial_link->ip_addr;

	// Read XB Serial from XBee module
	memset(xbsn,0xaa,XB_SN_LN);
	setXBsn();
	
	// Prepare packet fragmentation buffers
	memset(pkt0_frag,0xaa,MAX_XB_PYLD);
	memset(pkt1_frag,0xbb,MAX_XB_PYLD);
	pkt0_frag[PKT_FRAME] = 0x0;
	pkt1_frag[PKT_FRAME] = 0x1;
	memcpy(&pkt0_frag[PKT_XBSN],xbsn,XB_SN_LN);
	memcpy(&pkt1_frag[PKT_XBSN],xbsn,XB_SN_LN);

	next = 0;

	memset(frag_buffer,0,FRAG_BUF_SZ * RADIO_PKT_SZ);
	
	//Radio->write(pkt0_frag, MAX_XB_PYLD);
	//Radio->write(pkt1_frag, MAX_XB_PYLD);
	return;
}

bool SansgridRadio::rxComplete() {
	return true;
}

void SansgridRadio::setXBsn() {
	char xbsn_str[17];
	uint8_t * cmdOut = new uint8_t[8];

	memset(xbsn_str,'0',17);
	memset(cmdOut,0,8);
	xbsn_str[16] = '\0';
	
	// get char sn value
	atCmd(cmdOut,"ATSH");
	memcpy((xbsn_str+2),cmdOut,6); // hard code that sn starts with a 00 byte
	
	atCmd(cmdOut,"ATSL");
	memcpy((xbsn_str+8),cmdOut,8);

	//convert to hex
	atox(xbsn,xbsn_str,XB_SN_LN);
	
	// flush buffer and collect garbage 
	while(Radio->available() > 0) { Radio->read(); }
	delete xbsn_str;
	delete cmdOut;
}

void SansgridRadio::read() {
	int i = 0;
  while(Radio->available() > 0 && i < MAX_XB_PYLD) {
    delay(2);
    incoming_packet[i] = Radio->read();
		//Radio->write(incoming_packet[i]);
		i++;
  }
}

void SansgridRadio::set_mode(RadioMode mode) {
	router_mode = mode;
}

void SansgridRadio::write() {
  Radio->write(pending_packet,MAX_XB_PYLD);
}

void SansgridRadio::atCmd(uint8_t * result,const char * cmd) {
	int i = 0;
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
	
		Radio->println(cmd);

		delay(1200);
		uint8_t buffer[8];
		i = 0;
		while(Radio->available() > 0 && i < 8 ){//PACKET_SZ) {
		  buffer[i] = Radio->read();
			i++;
			delay(2);
		}	
		Radio->println();
		delay(100);
		Radio->println("ATCN");
		delay(1000);
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
	uint8_t uval;
	char offset;
	uint8_t adjust;

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
		//sscanf(chunk, "%x", &uval);
		if (chunk[0] >= 'A') {
			offset = 'A';
			adjust = 10;
		}
		else {
			offset = '0';
			adjust = 0;
		}
		if (chunk[1] != 0x00) {
			uval = ((chunk[0] - offset) + adjust);
		  uval = uval << 4;
			
			if (chunk[1] >= 'A') {
				offset = 'A';
				adjust = 10;
			}
			else {
				offset = '0';
				adjust = 0;
			}
			uval = uval | ((chunk[1] - offset) + adjust);
		}

		else {
			uval = ((chunk[0] - offset) + adjust);
			uval &= 0xf;
		}

		hexarray[i_hex] = (uval & 0xff);
		i_hex++;
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

