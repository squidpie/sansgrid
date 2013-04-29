/* Sensor Parse Payload implementation
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

#include <Arduino.h>
#include <SPI.h>
#include <sgSerial.h> 
#include <sensorPayloads.h>
#include <sensorConnect.h>
#include "sensorParse.h"

// Payloads recieved at Sensor from Router
void parseFly( SansgridSerial *rx , SansgridFly *sg_fly ){
	// TBD
}

void parsePeck( SansgridSerial *rx , SansgridPeck *sg_peck ){
	// TBD
}

void parseSing( SansgridSerial *rx , SansgridSing *sg_sing ){
	// TBD
}

void parseSquawk( SansgridSerial *rx , SansgridSquawk *sg_squawk ){
	// TBD
}

void parseNest( SansgridSerial *rx , SansgridNest *sg_nest ){
	// TBD
}

void parseHeartbeat( SansgridSerial *rx , SansgridHeartbeat *sg_heartbeat ){
    // TBD
}

void parseChirp( SansgridSerial *rx , SansgridChirp *sg_chirp ){
    // TBD
}
