/* Serial Communication API
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

#ifndef __SG_SERIAL_H__
#define __SG_SERIAL_H__

#include <stdint.h>
#include <arpa/inet.h>


// Send size bytes of serialdata serially
int8_t sgSerialSend(uint8_t *serialdata, uint32_t size);
// Get data from serial in. Data size will be in size.
int8_t sgSerialReceive(uint8_t *serialdata, uint32_t *size);



#endif

// vim: ft=c ts=4 noet sw=4:
