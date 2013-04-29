/* Sensor Connect to Network implementation 
 * specific to the Arduino DUE Platform
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

#ifndef __SENSOR_CONNECT_H__
#define __SENSOR_CONNECT_H__

#include <Arduino.h>
#include <sgSerial.h>
#include <sensorPayloads.h>

// Function connects sensor to network
void sensorConnect( SansgridSerial *tx , SensorConfig *sg_config );

// Functions prepare payloads and transmits payload across network
void transmitEyeball( SansgridSerial *tx , SansgridEyeball *sg_eyeball );
void transmitMock( SansgridSerial *tx , SansgridMock *sg_mock );
void transmitPeacock( SansgridSerial *tx , SansgridPeacock *sg_peacock );
void transmitSquawk( SansgridSerial *tx , SansgridSquawk *sg_squawk );
void transmitHeartbeat( SansgridSerial *tx , SansgridHeartbeat *sg_heartbeat);
void transmitChirp( SansgridSerial *tx , SansgridChirp *sg_chirp );

#endif // __SENSOR_CONNECT_H__
