#ifndef _H_SANSGRID_RADIO
#define _H_SANSGRID_RADIO

#include <Arduino.h>
//#include <SerialDebug.h>
#include "sgSerial.h"
#include "payloads.h"
#include "sgSnIp.h"

#define __FUNC__ (char *)__func__

#define SG_PACKET_SZ 89
#define IP_TABLE_SZ 12
#define PACKET_ID 0

#define PECK_TYPE 1
#define EYEBALL_TYPE 0


#define PECK_R_IP		1
#define PECK_A_IP 	17		
#define PECK_SN 		58
#define PECK_MANID 	50
#define PECK_MODID 	54

#define EYEBALL_SN 8


#define IP_LENGTH 16
#define SN_LENGTH 8


#define SPI_IRQ_PIN 8
#define ROUTER_MODE_PIN 12

#define XBSN	81

#define MAX_XB_PYLD 50
#define XB_SN_LN 8
#define PACKET_HEADER_SZ XB_SN_LN + 1
#define F0_PYLD_SZ 41
#define F1_PYLD_SZ 40
#define RADIO_PKT_SZ 90
#define FRAG_BUF_SZ 6

#define MODE(mode) (router_mode == mode)
#define IS_OK(err) (((sizeof(err) > 1) && (*err == 0x4F) && (*(err + 1) == 0x4B))) // check that err = 'OK'

#define BROADCAST 0x000000000000FFFF

#define PKT_FRAME 0
#define	PKT_XBSN  1
#define PKT_PYLD  9

enum RadioMode {
	SENSOR,
	ROUTER
};

enum FragTableEntry {
	FRAG_PENDING = 0,
	FRAG_SN = 1,
	FRAG_F0 = 9,
	FRAG_F1 = 50
};

enum packetIndex {
};

//static SerialDebug * debugger;
//void sgDebugInit(SerialDebug * db);
int btoi(byte * b,int ln);

void atox(uint8_t *hexarray, char *str, uint32_t hexsize);

void write_spi();
void read_spi();


class SansgridRadio {
	private:
		void setXBsn(void);
		int findSn(int sn);
		void atCmd(uint8_t *,const char *);
		bool mode(enum RadioMode mode);
		bool setDestAddr(uint64_t addr);
		uint8_t * genDevKey(uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn);
	// declare this here or everything breaks... wtf.	
		uint8_t incoming_packet[MAX_XB_PYLD];
		uint8_t pkt0_frag[MAX_XB_PYLD];
		uint8_t pkt1_frag[MAX_XB_PYLD];
		
		uint8_t * pending_packet;
		
		uint8_t * payload;
		uint8_t * ip;
		
		SansgridSerial * SpiData;
		HardwareSerial * Radio;
		SnIpTable * sn_table;
		
		RadioMode router_mode;
	
		uint8_t packet_buffer[SG_PACKET_SZ];
		
		uint8_t frag_buffer[FRAG_BUF_SZ][RADIO_PKT_SZ];
		
		uint8_t xbsn[XB_SN_LN];
		uint8_t origin_xbsn[XB_SN_LN];

		unsigned int next;
		//SerialDebug debug;
	public:
		SansgridRadio();
		void read();
		void write();
		void set_mode(RadioMode mode);
		void init(HardwareSerial *, SansgridSerial *, SnIpTable *);
		bool rxComplete();
		void processSpi();
		void loadFrame(int frame = -1);
		void processPacket(void);
		bool defrag(void);
};


#endif
