/* Sensor Payload Handler
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

#ifndef __SENSOR_PAYLOAD_HANDLER_H__
#define __SENSOR_PAYLOAD_HANDLER_H__

#include <Arduino.h>
#include <sgSerial.h> 
#include <sensorPayloads.h>

// Payload Handler
int8_t payloadHandler( SansgridSerial *rx , SansgridSerial *tx , SensorConfig *sensor_config );

// Payloads sent from Sensor to Router 
int8_t eyeball( SansgridSerial *tx , SansgridEyeball *sg_eyeball );
int8_t mock( SansgridSerial *tx , SansgridMock *sg_mock );
int8_t peacock( SansgridSerial *tx , SansgridPeacock *sg_peacock );
int8_t squawk( SansgridSerial *tx , SansgridSquawk *sg_squawk ); 

// Payloads recieved at Sensor from Router
int8_t peck( SansgridSerial *rx , SansgridPeck *sg_peck );
int8_t sing( SansgridSerial *rx , SansgridSing *sg_sing );
int8_t nest( SansgridSerial *rx , SansgridNest *sg_nest );

#endif // __SENSOR_PAYLOAD_HANDLER_H__