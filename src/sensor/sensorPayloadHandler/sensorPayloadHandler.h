/* Sensor Payload Handler Interface
 * Specific to the Arduino Platform
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef __SENSOR_PAYLOAD_HANDLER_H__
#define __SENSOR_PAYLOAD_HANDLER_H__

#include <Arduino.h>
#include <sensorParse.h>
#include <sensorPayloads.h>
#include <sgSerial.h>

void payloadHandler( SensorConfig *sg_config , SansgridSerial *sg_serial );
void payloadHandlerB( SensorConfig *sg_config , SansgridSerial *sg_serial );
void authenticateServer( SensorConfig *sg_config , SansgridSquawk *sg_squawk );
bool authenticateSensor( SensorConfig *sg_config , SansgridSquawk *sg_squawk );
void sensorConnect( SensorConfig *sg_config , SansgridSerial *sg_serial );
int freeRam ( void ); 
int16_t countBits(uint8_t value);

#endif // __SENSOR_PAYLOAD_HANDLER_H__
