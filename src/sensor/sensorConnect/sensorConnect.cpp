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
#include "sensorConnect.h"

void sensorConnect( SansgridSerial *tx , SensorConfig *sg_config ){
	// TBD
}

void transmitEyeball( SensorConfig *sg_config , SansgridEyeball *sg_eyeball ){
	int value = CONTROL + IP_ADDRESS + DT + MANID + MODNUM + SN + PROFILE + MODE;
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_config->router_ip , IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_eyeball->dt , DT , SLAVE_SELECT );
	spiMasterTransmit( sg_eyeball->manid , MANID , SLAVE_SELECT );
	spiMasterTransmit( sg_eyeball->modnum , MODNUM , SLAVE_SELECT );
	spiMasterTransmit( sg_eyeball->sn , SN , SLAVE_SELECT );
	spiMasterTransmit( sg_eyeball->profile , PROFILE , SLAVE_SELECT );
	spiMasterTransmit( sg_eyeball->mode , MODE , SLAVE_SELECT );
	if( value < NUM_BYTES)
		spiMasterPadding( *sg_config->padding , value , SLAVE_SELECT );
}
void transmitMock( SensorConfig *sg_config , SansgridMock *sg_mock ){
	int value = CONTROL + IP_ADDRESS + DT + SENSOR_KEY;
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_config->router_ip , IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_mock->dt , DT , SLAVE_SELECT );
	spiMasterTransmit( sg_mock->sensor_public_key , SENSOR_KEY , SLAVE_SELECT );
	if( value < NUM_BYTES)
		spiMasterPadding( *sg_config->padding , value , SLAVE_SELECT );
}

void transmitPeacock( SensorConfig *sg_config , SansgridPeacock *sg_peacock ){
	int value = CONTROL + IP_ADDRESS + DT + SENSOR_ID + CLASSIFICATION + DIRECTION + 
		LABEL + UNITS + SENSOR_ID + CLASSIFICATION + DIRECTION + LABEL + UNITS + ADDITIONAL;
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_config->router_ip , IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_peacock->dt , DT , SLAVE_SELECT );
	transmitSensor( &sg_peacock->a );
	transmitSensor( &sg_peacock->b );
	spiMasterTransmit( sg_peacock->additional , ADDITIONAL , SLAVE_SELECT );
	if( value < NUM_BYTES)
		spiMasterPadding( *sg_config->padding , value , SLAVE_SELECT );
}

void transmitSquawk( SensorConfig *sg_config , SansgridSquawk *sg_squawk ){
	int value = CONTROL + IP_ADDRESS + DT + DATA;
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_config->router_ip, IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_squawk->dt , DT , SLAVE_SELECT );
	spiMasterTransmit( sg_squawk->data , DATA , SLAVE_SELECT );
	if( value < NUM_BYTES)
	    spiMasterPadding( *sg_config->padding , value , SLAVE_SELECT );
}

void transmitChirp( SensorConfig *sg_config , SansgridChirp *sg_chirp ){
	int value = CONTROL + IP_ADDRESS + DT + DATA;
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_config->router_ip , IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_chirp->dt , DT , SLAVE_SELECT );
	spiMasterTransmit( sg_chirp->data , DATA , SLAVE_SELECT );
	if( value < NUM_BYTES)
		spiMasterPadding( *sg_config->padding , value , SLAVE_SELECT );
}

void transmitHeartbeat( SensorConfig *sg_config , SansgridHeartbeat *sg_heartbeat){
	int value = CONTROL + IP_ADDRESS + DT;
	spiMasterTransmit( sg_config->control , CONTROL , SLAVE_SELECT );
	spiMasterTransmit( sg_config->router_ip , IP_ADDRESS , SLAVE_SELECT );
	spiMasterTransmit( sg_heartbeat->dt , DT , SLAVE_SELECT );
	if( value < NUM_BYTES)
		spiMasterPadding( *sg_config->padding , value , SLAVE_SELECT );
}

void transmitSensor( SansgridSensor *sg_sensor){
	spiMasterTransmit( sg_sensor->id , SENSOR_ID , SLAVE_SELECT );
	spiMasterTransmit( sg_sensor->classification , CLASSIFICATION , SLAVE_SELECT );
	spiMasterTransmit( sg_sensor->direction , DIRECTION , SLAVE_SELECT );
	spiMasterTransmit( sg_sensor->label , LABEL , SLAVE_SELECT );
	spiMasterTransmit( sg_sensor->units , UNITS , SLAVE_SELECT );
}
