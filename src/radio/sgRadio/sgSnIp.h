#ifndef _H_SANSGRID_SNIP
#define _H_SANSGRID_SNIP

#include <Arduino.h>


#define SNIPEXPANDFACTOR 1.5
#define DEFAULTSNIPSIZE 1
#define SNIPTABLEWIDTH	2
#define SNIPBYTEWIDTH		8

enum SnTableIndex {
	IP,
	SN,
};

typedef struct {
	uint8_t ip[SNIPBYTEWIDTH];
	uint8_t sn[SNIPBYTEWIDTH];
}SnIpEntry;

class SnIpTable {
	private:
		SnIpEntry * table;
		int next;
		int size;
		void snipExpand(void);
		void snipInsert(uint8_t * data, int index, SnTableIndex type = SN);
		int snipFindKey(uint8_t *);
	public:
		SnIpTable();
		~SnIpTable();
		
		int snipFindSn(uint8_t *);
		int snipFindIp(uint8_t *);
		void snipRemove(uint8_t * ip);	
		uint64_t snipSnFromIp(uint8_t *);
		uint64_t snipIpFromSn(uint8_t *);
		void snipInsertIp(uint8_t * ip, uint8_t * key);
		void snipInsertIp(uint8_t * ip, int index);
		void snipInsertSn(uint8_t * sn, uint8_t * key);
		void snipInsertSn(uint8_t * sn, int index);
};

#endif
