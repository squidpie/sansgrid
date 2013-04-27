#ifndef _H_SANSGRID_RADIO
#define _H_SANSGRID_RADIO

#include <Arduino.h>
#include <SerialDebug.h>
#include <sgSerial.h>
#include <payloads.h>

#define __FUNC__ (char *)__func__

#define PACKET_SZ 100
#define IP_TABLE_SZ 12
#define PACKET_ID 0

#define PECK_TYPE 1
#define EYEBALL_TYPE 0

#define PECKIP 1
#define PECKSN 				57
#define PECKMANID 		1
#define PECKMODID 		5

#define EYE_SN_ENTRY 8

#define IP_LENGTH 16
#define SN_LENGTH 8


#define SPI_IRQ_PIN 8
#define ROUTER_MODE_PIN 12

#define SNIPEXPANDFACTOR 1.5
#define DEFAULTSNIPSIZE 1
#define SNIPTABLEWIDTH	2
#define SNIPBYTEWIDTH		8

#define XBSN	81

enum RadioMode {
	SENSOR,
	ROUTER
};

enum SnTableIndex {
	IP,
	SN,
};

static SerialDebug * debugger;
void sgDebugInit(SerialDebug * db);
int btoi(byte * b,int ln);

void write_spi();
void read_spi();

typedef struct {
	uint8_t ip[SNIPBYTEWIDTH];
	uint8_t sn[SNIPBYTEWIDTH];
}SnIpEntry;

class SnIpTable {
	private:
		SnIpEntry * table;
		int next;
		int size;
		void snIpExpand(void);
		void snIpInsert(uint8_t * data, int index, SnTableIndex type = SN);
	public:
		SnIpTable();
		~SnIpTable();
		int snIpFindSn(uint8_t *);
		int snIpFindIp(uint8_t *);
		void snIpInsertIp(uint8_t * ip, uint8_t * key);
		void snIpInsertIp(uint8_t * ip, int index);
		void snIpInsertSn(uint8_t * sn);
		void snIpInsertSn(uint8_t * sn, int index);
};


class SansgridRadio {
	private:
		RadioMode router_mode;
		uint8_t packet_buffer[PACKET_SZ];
		uint8_t * packet;
		int findSn(int sn);
		void processPacket(void);
		void atCmd(char *,const char *);
		uint8_t * genDevKey(uint8_t * man_id, uint8_t * mod_id, uint8_t * dev_sn);
		SnIpTable * sn_table;
		SerialDebug debug;
		HardwareSerial * Radio;
	public:
		SansgridRadio(HardwareSerial *,SansgridSerial *, SnIpTable *);
		~SansgridRadio();
		void read();
		void write();
		void set_mode(RadioMode mode);
		void test();
};



#endif
