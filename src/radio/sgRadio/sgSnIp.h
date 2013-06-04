/* sansgridRadio Route over XBee Routing Table
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
 
#ifndef _H_SANSGRID_SNIP
#define _H_SANSGRID_SNIP

#include <Arduino.h>


#define SNIPEXPANDFACTOR 1.5
#define DEFAULTSNIPSIZE 5
#define SNIPTABLEWIDTH	2
#define SNIPBYTEWIDTH	8
#define SNIP_XB_WIDTH 8
#define SNIP_IP_WIDTH 16


enum SnTableIndex {
	IP,
	SN,
	DEV
};

typedef struct {
	uint8_t ip[SNIP_IP_WIDTH];
	uint8_t sn[SNIP_XB_WIDTH];
	uint8_t dev_sn[SNIP_XB_WIDTH];
}SnIpEntry;

class SnIpTable {
	private:
		SnIpEntry table[DEFAULTSNIPSIZE];
		int next;
		int size;
		void snipExpand(void);
		void snipInsert(uint8_t * data, int index, SnTableIndex type);
		int snipFindKey(uint8_t *);
	public:
		SnIpTable();
		~SnIpTable();
		int snipGetIndex();
		void snipDebug(HardwareSerial * );
		int snipFindSn(uint8_t *);
		int snipFindIp(uint8_t *);
		int snipFindDevSn(uint8_t *);
		void snipRemove(uint8_t * sn);	
		void snipSnFromIp(uint8_t * sn, uint8_t * ip);
		void snipIpFromSn(uint8_t * ip, uint8_t * sn);
		int snipInsertIp(uint8_t * ip, uint8_t * dev);
	//	void snipInsertIp(uint8_t * ip, int index);
		int snipInsertSn(uint8_t * sn, uint8_t * dev);
//		void snipInsertSn(uint8_t * sn, int index);
};

#endif
