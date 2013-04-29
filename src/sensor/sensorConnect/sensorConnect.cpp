/* Sensor Connect to Sansgrid Network implementation
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
#include <sensorPayloads.h>
#include <spiMaster.h>
#include "sensorConnect.h"


// Function connects sensor to network
void sensorConnect( SansgridSerial *tx , SensorConfig *sg_config ){
    // TBD
}

// Functions prepare payloads and transmits payload across network
void transmitEyeball( SansgridSerial *tx , SansgridEyeball *sg_eyeball ){
	// TBD
}
void transmitMock( SansgridSerial *tx , SansgridMock *sg_mock ){
    // TBD
}

void transmitPeacock( SansgridSerial *tx , SansgridPeacock *sg_peacock ){
	// TBD
}

void transmitSquawk( SansgridSerial *tx , SansgridSquawk *sg_peacock ){
	// TBD
}

void transmitChirp( SansgridSerial *tx , SansgridPeacock *sg_chirp ){
	// TBD
}

void transmitHeartbeat( SansgridSerial *tx , SansgridPeacock *sg_heartbeat){
	// TBD
}
