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
#include "sensorParse.h"

// Payloads recieved at Sensor from Router
void parseFly( SansgridSerial *rx , SansgridFly *sg_fly ){
	memcpy( sg_fly->dt , rx->payload , DT );
	memcpy( sg_fly->network_name , rx->payload + 1 , DATA );
}

void parsePeck( SansgridSerial *rx , SansgridPeck *sg_peck ){
	memcpy( sg_peck->dt , rx->payload , DT );
	memcpy( sg_peck->router_ip , rx->payload + DT , IP_ADDRESS );
	memcpy( sg_peck->ip_address , rx->payload + DT + IP_ADDRESS , IP_ADDRESS );
	memcpy( sg_peck->server_id , rx->payload + DT + IP_ADDRESS + IP_ADDRESS , SERVER_ID );
	memcpy( sg_peck->recognition , rx->payload + DT + IP_ADDRESS + IP_ADDRESS + SERVER_ID , RECOGNITION ); 
	memcpy( sg_peck->manid , rx->payload + DT + IP_ADDRESS + IP_ADDRESS + SERVER_ID + RECOGNITION , MANID );
	memcpy( sg_peck->modnum, rx->payload + DT + IP_ADDRESS + IP_ADDRESS + SERVER_ID + RECOGNITION + MANID , MODNUM );
	memcpy( sg_peck->sn , rx->payload + DT + IP_ADDRESS + IP_ADDRESS + SERVER_ID + RECOGNITION + MANID + MODNUM , SN );
}

void parseSing( SansgridSerial *rx , SansgridSing *sg_sing ){
	memcpy( sg_sing->dt , rx->payload , DT );
	memcpy( sg_sing->server_public_key , rx->payload + DT , SERVER_KEY );
}

void parseSquawk( SansgridSerial *rx , SansgridSquawk *sg_squawk ){
	memcpy( sg_squawk->dt , rx->payload , DT );
	memcpy( sg_squawk->data , rx->payload + DT , DATA );
}

void parseNest( SansgridSerial *rx , SansgridNest *sg_nest ){
	memcpy( sg_nest->dt , rx->payload , DT );
}

void parseHeartbeat( SansgridSerial *rx , SansgridHeartbeat *sg_heartbeat ){
	memcpy( sg_heartbeat->dt , rx->payload , DT );
}

void parseChirp( SansgridSerial *rx , SansgridChirp *sg_chirp ){
	memcpy( sg_chirp->dt , rx->payload , DT );
	memcpy( sg_chirp->data , rx->payload + DT , DATA );
}
