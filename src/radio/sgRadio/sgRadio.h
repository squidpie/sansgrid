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
/// \file sgRadio
 
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

#define SG_ACK_SZ 9
#define ACK 0xAC


/*! RadioMode Enum
	Marker for Radio in Sensor or Router mode
*/
enum RadioMode {
	SENSOR,
	ROUTER
};

/*!	\class SansgridRadio
	\brief SansgridRadio Arduino Object
	The SansgridRadio class is the main interface for the Arduino Sketch
	Requires a call to SansgridRadio::init during setup
*/	
class SansgridRadio {
	private:
		/** setXBsn 
			\brief Stores the Serial Number of the currently connected XBee Radio
			\param void
			\returns void
		*/
		void setXBsn(void);
		
		int findSn(int sn);
		
		/** atCmd
			\brief Utility function to execute an AT Command on the XBee Radio
			\param[out] result	byte array returned from cmd output
			\param[in] cmd		char array of cmd to execute
			\returns void
		*/
		void atCmd(uint8_t *,const char *);
		
		bool mode(enum RadioMode mode);
		bool setDestAddr(uint64_t addr);
		void genDevKey(uint8_t * key, uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn);
		
		/** on_network flag
			\brief Flag is set on receipt of Nest
			Cleared on timeout and Drop Sensor Packets
		*/
		bool on_network;
	
		/** incoming_packet buffer
			\brief byte array to store a packet coming from the XBee Radio
		*/
		uint8_t incoming_packet[MAX_XB_PYLD];
		
		/** packet_buffer
			\brief byte array for transfering a packet between functions
		*/
		uint8_t packet_buffer[SG_PYLD_SZ];
		
		/** frag_buffer
			\brief byte array of currently pending fragments
		*/
		uint8_t frag_buffer[FRAG_BUF_SZ][RADIO_PKT_SZ];
		
		/** pkt0_frag
			\brief byte array for storing outgoing fragment 0
		*/
		uint8_t pkt0_frag[MAX_XB_PYLD];
		
		/** pkt1_frag
			\brief byte array for storing outgoing fragment 1
		*/
		uint8_t pkt1_frag[MAX_XB_PYLD];
		
		/** pending_packet
			\brief byte pointer for storing head of next outgoing packet
		*/
		uint8_t * pending_packet;
	
		/** payload
			\brief byte pointer to head of SansgridSerial payload section
		*/
		uint8_t * payload;
		
		/** ip
			\brief byte pointer to head of SansgridSerial ip section
		*/
		uint8_t * ip;
		
		/** SpiData
			\brief SansgridSerial pointer to Spi descriptor
			memory allocated in main sketch
		*/
		SansgridSerial * SpiData;
		
		/** Radio
			\brief HardwareSerial pointer to Radio interface
			memory allocated in main sketch
		*/
		HardwareSerial * Radio;
		
		/** sn_table
			\brief SnIpTable pointer to Router-Over-Xbee routing table
			memory allocated in main sketch
		*/
		SnIpTable * sn_table;
		
		/** router_mode
			\brief RadioMode enum indicating current operating mode
		*/
		RadioMode router_mode;
	
		/** router_ip
			\brief byte array storing router ip address
		*/
		uint8_t router_ip[IP_LENGTH];
		
		uint8_t self_ip[IP_LENGTH];
		
		/** xbsn
			\brief byte array storing XBee serial number
		*/
		uint8_t xbsn[XB_SN_LN];
		
		/** origin_xbsn
			\brief byte array storing XBee serial number of incoming packet
		*/
		uint8_t origin_xbsn[XB_SN_LN];
		
		unsigned int next;
		
		/** timout_counter
			\brief rollover timer to indicate loss of network connectivity
		*/
		uint32_t timeout_counter;
		
	public:
		/** Sansgrid constructor
			\brief not currently doing anything, portions of init may be moved here
			\param void
		*/
		SansgridRadio();
		
		/** read()
			\brief reads a packet fragment from the XBee radio
			\param void
		*/
		int read();
		
		/** write()
			\brief writes pending_packet to XBee radio
			\param void
		*/
		void write();
		
		/** set_mode()
			\brief sets Radio Mode ot SENSOR or ROUTER
			\param[in] mode		RadioMode enum  
		*/
		void set_mode(RadioMode mode);
		
		/** init()
			\brief Initialize SansgridRadio class
			\param[in] xbee_link	HardwwareSerial pointer
			\param[in] serial_link	SansgridSerial pointer
			\param[in] table_link	SnIpTable pointer
		*/
		void init(HardwareSerial *, SansgridSerial *, SnIpTable *);
		
		bool rxComplete();
		
		/** processSpi()
			\brief processes outbound packet from Spi
			\param void
		*/
		void processSpi();
		
		/** loadFrame()
			\brief loads a packet fragment frame into outbound pending 
			\param[in] frame	frame number to load
		*/
		void loadFrame(int frame = -1);
		
		/** processPacket()
			\brief processing inbound packet from XBee
			\param void
		*/
		bool processPacket(void);
		
		/** defrag()
			\brief processing an inbound fragement and generates complete packet
			\param void
			\returns bool	on complete packet ready, true else false
		*/
		bool defrag(void);
		
		/** timeout()
			\brief increments timeout rollover, returns true if timeout has occured
			acts as a watchdog timer for the Radio being dropped silently from the network
			\param void
			\returns bool	on timeout, true else false 
		*/
		bool timeout();
};

// Global Helper functions
/**  btoi()
	\brief convert byte pointer of length ln into int
	\param[in] b	byte pointer
	\param[in] ln	length of byte array
	\returns int
*/
int btoi(byte * b,int ln);

/** atox()
	\brief converts ascii string to hex byte array
	\param[out] hexarray	byte array to store converted results
	\param[in] str			char array to convert
	\param[in] hexsize		size of output byte array
	\returns void
*/
void atox(uint8_t *hexarray, char *str, uint32_t hexsize);



#endif
