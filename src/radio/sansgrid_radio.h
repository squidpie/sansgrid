#ifndef _H_SANSGRID_RADIO
#define _H_SANSGRID_RADIO

#include <Arduino.h>
#include <SerialDebug.h>

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
#define IP 0
#define SN 1

#define SPI_IRQ_PIN 8
#define ROUTER_MODE_PIN 12

enum radio_mode {
	SENSOR = 0,
	ROUTER = 1,
};

static byte packet_buffer[PACKET_SZ];
static SerialDebug debugger;
void sansgrid_debug_init(SerialDebug &db);
int byte2int(byte * b,int ln);

void write_spi();
void read_spi();

class sansgrid_radio {
	private:
		int ip_table[IP_TABLE_SZ][2];
		radio_mode router_mode;
		SerialDebug debug;
		int next_sn_entry();
		int find_pending_ip(int sn);
		void process_packet();
		void at_cmd(char *,const char *);
		HardwareSerial * radio;
	public:
		sansgrid_radio();
		~sansgrid_radio();
		void read();
		void write();
		void set_mode(radio_mode mode);
};



#endif
