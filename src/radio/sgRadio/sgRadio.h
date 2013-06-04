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

#ifndef _H_SANSGRID_RADIO
#define _H_SANSGRID_RADIO

//#define SG_PACKET_SZ 89
//#define PACKET_ID 0
//#define PECK_TYPE 1
//#define EYEBALL_TYPE 0

#include <Arduino.h>
//#include "../Timer/Timer.h"
#include "sgSerial.h"
#include "payloads.h"
#include "sgSnIp.h"

#define __FUNC__ (char *)__func__

#define IP_TABLE_SZ 12

#define PECK_R_IP		1
#define PECK_A_IP 		17		
#define PECK_RECOG		49
#define PECK_MANID 		50
#define PECK_MODID 		54
#define PECK_SN 		58

#define EYEBALL_MANID 	1	
#define EYEBALL_MODID 	5
#define EYEBALL_SN 		9
#define EYEBALL_MODE 	17

#define SPI_IRQ_PIN 8
#define ROUTER_MODE_PIN 9

#define IP_LENGTH 16
#define XB_SN_LN 8

#define PACKET_HEADER_SZ XB_SN_LN + 1
#define F0_PYLD_SZ 41
#define F1_PYLD_SZ 40
#define MAX_XB_PYLD 50
#define RADIO_PKT_SZ 90
#define SG_PYLD_SZ 81
#define FRAG_BUF_SZ 6

#define MODE(mode) (router_mode == mode)
#define IS_OK(err) (((sizeof(err) > 1) && (*err == 0x4F) && (*(err + 1) == 0x4B))) // check that err = 'OK'

#define PKT_FRAME 0
#define	PKT_XBSN  1
#define PKT_PYLD  9

// This was an enum...
//enum packetIndex {
//};
#define FRAG_PENDING  	0
#define FRAG_SN  		1
#define FRAG_F0  		9
#define FRAG_F1  		50

enum RadioMode {
	SENSOR,
	ROUTER
};

//static Timer write_clear;
//static bool write_lock;
//static void write_unlock();

// Sansgrid Radio Class Object
class SansgridRadio {
	private:
		void setXBsn(void);
		int findSn(int sn);
		void atCmd(uint8_t *,const char *);
		bool mode(enum RadioMode mode);
		bool setDestAddr(uint64_t addr);
		void genDevKey(uint8_t * key, uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn);
		bool on_network;
	
		uint8_t incoming_packet[MAX_XB_PYLD];
		uint8_t packet_buffer[SG_PYLD_SZ];
		
		uint8_t frag_buffer[FRAG_BUF_SZ][RADIO_PKT_SZ];
		uint8_t pkt0_frag[MAX_XB_PYLD];
		uint8_t pkt1_frag[MAX_XB_PYLD];
		
		uint8_t * pending_packet;
	
		uint8_t * payload;
		uint8_t * ip;
		SansgridSerial * SpiData;
		HardwareSerial * Radio;
		SnIpTable * sn_table;
		
		RadioMode router_mode;
	
		uint8_t router_ip[IP_LENGTH];
		uint8_t self_ip[IP_LENGTH];
		uint8_t xbsn[XB_SN_LN];
		uint8_t origin_xbsn[XB_SN_LN];
		unsigned int next;
		unsigned int timeout;
		
	public:
		SansgridRadio();
		int read();
		void write();
		void set_mode(RadioMode mode);
		void init(HardwareSerial *, SansgridSerial *, SnIpTable *);
		bool rxComplete();
		void processSpi();
		void loadFrame(int frame = -1);
		bool processPacket(void);
		bool defrag(void);
};

// Global Helper functions
int btoi(byte * b,int ln);

void atox(uint8_t *hexarray, char *str, uint32_t hexsize);



#endif
