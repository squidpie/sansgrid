/* sansgridRadio Arduino sgRadio interface object
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
	uint8_t output_sn[XB_SN_LN];
	uint8_t key[IP_LENGTH];
	int index;
	//Radio->println("Process Spi"); 
	if (payload == NULL) {
		Radio->println("Payload is invalid");
		delay(50);
	}

	memcpy(&type,payload,1);
	
	switch (payload[0]) {
		case SG_FLY:
			if(MODE(ROUTER)) {
				// Set Destination to Broadcast
				memset(output_sn, 0, XB_SN_LN);
			}
			break;
		case SG_PECK:
			if(MODE(ROUTER)) {
				// Set Destination to Broadcast
				// store IP at device key for XBsn
				index = sn_table->snipInsertIp((payload + PECK_A_IP), (payload + PECK_SN));
			}
			memset(output_sn, 0, XB_SN_LN);
			break;
		case SG_HATCH: 
			// router ip received
			if (MODE(SENSOR)) { 
				router_mode = ROUTER;
			}	
			// Set destination to invalid so that receiving sensors discard packet, should maybe make this just not send, but this hack works for now.
			memset(output_sn, 0xFE, XB_SN_LN);
			break;
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			if (MODE(ROUTER)) {
				sn_table->snipSnFromIp(output_sn, ip);
				sn_table->snipRemove(output_sn);
				}
			break;
		case SG_CHIRP_SENSOR_DISCONNECT:
			if (MODE(SENSOR)) {
				on_network = false;
				memset(router_ip, 0, IP_LENGTH);
				memcpy(output_sn, xbsn, XB_SN_LN);
			}
			break;
		default:
			if (MODE(ROUTER)) {
				//address a specific sensor XBee module
				sn_table->snipSnFromIp(output_sn, ip);
			}
			else {
				// Identify orginating sensor XBee module
				memcpy(output_sn, xbsn, XB_SN_LN);
			}
			break;
	}
	
	// Prep fragment memory for transmission
	memcpy(&pkt0_frag[1], output_sn, XB_SN_LN);
	memcpy(&pkt0_frag[PACKET_HEADER_SZ],payload,F0_PYLD_SZ);

	memcpy(&pkt1_frag[1], output_sn, XB_SN_LN);
	memcpy(&pkt1_frag[PACKET_HEADER_SZ],(payload+F0_PYLD_SZ),F1_PYLD_SZ);
}

bool SansgridRadio::defrag() {
	bool rv = false;
	int insert_pos = next;
	if (incoming_packet[PKT_FRAME] == 0x00) {
		for (int i = 0; i < FRAG_BUF_SZ; i++) {
			if (frag_buffer[i][FRAG_PENDING] && !memcmp(&frag_buffer[i][FRAG_SN], &incoming_packet[PKT_XBSN], XB_SN_LN)) {
				insert_pos = i;
				break;
			}
		}
		frag_buffer[insert_pos][FRAG_PENDING] = 1;
		memcpy(&frag_buffer[insert_pos][FRAG_SN],&incoming_packet[PKT_XBSN],XB_SN_LN);
		memcpy(&frag_buffer[insert_pos][FRAG_F0],&incoming_packet[PKT_PYLD],F0_PYLD_SZ);
		if ((insert_pos == next) && (++next) == FRAG_BUF_SZ) next = 0;
	}
	else if (incoming_packet[PKT_FRAME] == 0x01) {
		for (int i = 0; i < FRAG_BUF_SZ; i++) {
			if (frag_buffer[i][FRAG_PENDING] && !memcmp(&frag_buffer[i][FRAG_SN],&incoming_packet[PKT_XBSN],XB_SN_LN)){
				frag_buffer[i][FRAG_PENDING] = 0;
				memcpy(&frag_buffer[i][FRAG_F1], &incoming_packet[PKT_PYLD],F1_PYLD_SZ);
				for (int k = 0; k < SG_PYLD_SZ; k++) {
					packet_buffer[k] = frag_buffer[i][FRAG_F0 + k];
				}
				//memcpy(&packet_buffer[0], &frag_buffer[i][FRAG_F0],SG_PYLD_SZ);
				//memcpy(packet_buffer, &frag_buffer[i][FRAG_F0],F0_PYLD_SZ);
				//memcpy(&packet_buffer[F0_PYLD_SZ], &incoming_packet[PKT_PYLD], F1_PYLD_SZ);
				memcpy(origin_xbsn, &frag_buffer[i][FRAG_SN], XB_SN_LN); // TODO check for correctness
				rv = true;
				break;
			}
		}
	}
	return rv;
}

bool SansgridRadio::processPacket() {
	bool send_packet = true;
	int index;
	timeout_counter = 1;			
	switch (packet_buffer[0]) {
		case SG_HEARTBEAT_ROUTER_TO_SENSOR:
			send_packet = false;
			if (on_network) {
				memset(packet_buffer, 0, PAYLOAD_SIZE);
				packet_buffer[0] = SG_HEARTBEAT_SENSOR_TO_ROUTER;
			}
			break;
		case SG_HATCH:
			send_packet = false;
			break;
		case SG_FLY:
			if (MODE(SENSOR) && (!on_network)) {
				memset(ip, 0xFF, IP_LENGTH);
			}
			else {
				send_packet = false;
			}
			break;
		case SG_NEST:
			if(MODE(SENSOR) && !on_network) {
				on_network = true;
				memcpy(ip, router_ip, IP_LENGTH);
			}
			else {
				send_packet = false;
			}
			break;
		case SG_PECK:
			if(MODE(SENSOR) && !on_network) {
				memcpy(router_ip, &packet_buffer[PECK_R_IP], IP_LENGTH);
				memset(ip, 0xFF, IP_LENGTH);
			}
			else {
				send_packet = false;
			}
			break;
		case SG_EYEBALL:
			if(MODE(ROUTER)) {
				index = sn_table->snipInsertSn(origin_xbsn, (packet_buffer + EYEBALL_SN));
			}
			memset(ip, 0xFF, IP_LENGTH);
			break;
		case SG_CHIRP_NETWORK_DISCONNECTS_SENSOR:
			if (MODE(SENSOR)) {
				on_network = false;
				memcpy(ip, router_ip, IP_LENGTH);
				memset(router_ip, 0, IP_LENGTH);
			}
			else {
				send_packet = false;
			}
			break;
		case SG_CHIRP_SENSOR_DISCONNECT:
			if (MODE(ROUTER)) {
				sn_table->snipIpFromSn(ip, origin_xbsn);
				sn_table->snipRemove(origin_xbsn);
			}
			else {
				send_packet = false;
			}
			break;
		default:
			if (MODE(ROUTER)) {
				sn_table->snipIpFromSn(ip, origin_xbsn);
			}
			else {
				memcpy(ip, router_ip, IP_LENGTH);
			}
			break;
	}
	SpiData->control = SG_SERIAL_CTRL_VALID_DATA;
	memcpy(SpiData->payload, packet_buffer, SG_PYLD_SZ);
	return send_packet;
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

bool SansgridRadio::timeout(void) {
	bool timeout_flag = false;
	if (!on_network || MODE(ROUTER)) return timeout_flag;
	timeout_counter++;
	if (timeout_counter == 0) {
		timeout_flag = true;
		SpiData->control = SG_SERIAL_CTRL_VALID_DATA;
		memcpy(ip, router_ip, IP_LENGTH);
		memset(router_ip, 0, IP_LENGTH);
		SpiData->payload[0] = SG_CHIRP_NETWORK_DISCONNECTS_SENSOR;
		on_network = false;
	}
	return timeout_flag;
}

void SansgridRadio::init(HardwareSerial * xbee_link, SansgridSerial * serial_link, SnIpTable * table_link) {
	Radio = xbee_link;
	sn_table = table_link;
	SpiData = serial_link;

	payload = serial_link->payload;
	ip = serial_link->ip_addr;

	// Read XB Serial from XBee module
	memset(xbsn,0x0,XB_SN_LN);
	setXBsn();
	// Prepare packet fragmentation buffers
	memset(pkt0_frag,0x0,MAX_XB_PYLD);
	memset(pkt1_frag,0x0,MAX_XB_PYLD);
	pkt0_frag[PKT_FRAME] = 0x0;
	pkt1_frag[PKT_FRAME] = 0x1;
	memcpy(&pkt0_frag[PKT_XBSN],xbsn,XB_SN_LN);
	memcpy(&pkt1_frag[PKT_XBSN],xbsn,XB_SN_LN);
	memset(frag_buffer,0,FRAG_BUF_SZ * RADIO_PKT_SZ);
	
	on_network = false;
	next = 0;
	timeout_counter = 1;	
	
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
	
	delete cmdOut;
}

int SansgridRadio::read() {
	int i = 0;
	//while(Radio->available() > 0 && i < MAX_XB_PYLD) {
	while(Radio->available() && i < MAX_XB_PYLD) {
		delay(2);
		incoming_packet[i] = Radio->read();
		i++;
	}
	
	if (MODE(SENSOR)) {
		uint8_t brdcst[XB_SN_LN];
		memset(brdcst,0x0,XB_SN_LN);
		if ((memcmp(&incoming_packet[1],xbsn,XB_SN_LN) != 0) && (memcmp(&incoming_packet[1],brdcst,XB_SN_LN) != 0))  {
			return 0;
		}
	}
	return i;
}

void SansgridRadio::set_mode(RadioMode mode) {
	router_mode = mode;
}


// Write pending packet to the radio
// Each write REQUIRES a 500ms delay between packets.
// To meet this requirement, delay 250ms at head and tail of function
void SansgridRadio::write() {
	delay(250);
	Radio->write(pending_packet, MAX_XB_PYLD);
	delay(250);
}

// XBee service function to enter AT Mode and issue provided command
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


void SansgridRadio::genDevKey(uint8_t * key, uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn) {

	memcpy(key, dev_sn, 8);
	memcpy((key + 8), mod_id, 4);
	memcpy((key + 12), man_id, 4);
/*
	for (int i = 0; i < SNIPBYTEWIDTH/2; i+=2) {
		key[i] = man_id[i];
		key[i+1] = mod_id[i];
	}
	for (int i = 0; i < SNIPBYTEWIDTH; i++) {
		key[i] ^= dev_sn[i];
	}
*/
	return;
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

