#ifndef _H_SANSGRID_RADIO
#define _H_SANSGRID_RADIO

#include <Arduino.h>
#include <SerialDebug.h>
#include <sgSerial.h>

#define __FUNC__ (char *)__func__

#define PACKET_SZ 100
#define IP_TABLE_SZ 12
#define PACKET_ID 0
#define PECK_TYPE 1
#define EYEBALL_TYPE 0

#define PECK_IP_ENTRY  1
#define PECK_SN_ENTRY 57
#define EYE_SN_ENTRY 8
#define IP_LENGTH 16
#define SN_LENGTH 8
//#define IP 0
//#define SN 1

#define SPI_IRQ_PIN 8
#define ROUTER_MODE_PIN 12

#define SNIPEXPANDFACTOR 1.5

enum RadioMode {
	SENSOR,
	ROUTER
};

enum SnTableIndex {
	IP,
	SN
};

static byte packet_buffer[PACKET_SZ];
static SerialDebug debugger;
void sgDebugInit(SerialDebug &db);
int btoi(byte * b,int ln);

void write_spi();
void read_spi();

typedef struct {
	uint64_t ip;
	uint64_t sn;
}SnIpEntry;

class SnIpTable {
	SnIpEntry * table;
	int next;
	int size;
	void snIpExpand(void);
	void snIpInsert(uint64_t data, int index, SnTableIndex type = SN);
	int snIpfindSn(uint64_t);
	int snIpfindIp(uint64_t);
	void snIpInsertIp(uint64_t ip, uint64_t key);
	void snIpInsertIp(uint64_t ip, int index);
	void snIpInsertSn(uint64_t sn, int index);
};


class SansgridRadio {
	private:
		RadioMode router_mode;
		uint8_t packet_buffer[PACKET_SZ];
		uint8_t * packet;
		int getEmptyIndex(void);
		int findSn(int sn);
		void processPacket(void);
		void atCmd(char *,const char *);
		uint8_t * genDevKey(void);
		SnIpTable * sn_table;
		SerialDebug debug;
		HardwareSerial * Radio;
	public:
		SansgridRadio(SansgridSerial *, SnIpTable *);
		~SansgridRadio();
		void read();
		void write();
		void set_mode(RadioMode mode);
};



#endif
