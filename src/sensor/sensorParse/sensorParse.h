/* Sensor Parse Payload interface
 * Specific to the Arduino DUE Platform
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

#ifndef __SENSOR_PARSE_H__
#define __SENSOR_PARSE_H__

#include <Arduino.h>
#include <sgSerial.h> 
#include <sensorPayloads.h>

// Payloads sent from Sensor to Router 
int8_t parseFly( SansgridSerial *tx , SansgridEyeball *sg_fly );
int8_t parsePeck( SansgridSerial *rx , SansgridPeck *sg_peck );
int8_t parseSquawk( SansgridSerial *tx , SansgridSquawk *sg_squawk ); 
int8_t parseSing( SansgridSerial *rx , SansgridSing *sg_sing );
int8_t parseNest( SansgridSerial *rx , SansgridNest *sg_nest );

#endif // __SENSOR_PARSE_H__
