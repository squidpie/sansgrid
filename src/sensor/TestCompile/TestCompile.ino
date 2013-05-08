#include <sensorPayloadHandler.h>
#include <sgSerial.h>
#include <spiMaster.h>
#include <sensorPayloads.h>
#include <sensorParse.h>
#include <sensorConnect.h>
#include <SPI.h>

SensorConfig g;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly: 
  Serial.begin( 9600 );
  for( int i = 0 ; i < MANID ; i++)
      Serial.println( g.manid[i] ); 
  Serial.println( '\n' );
  for( int i = 0 ; i < MODNUM ; i++)
      Serial.println( g.modnum[i] );
  Serial.println( '\n' );
  for( int i = 0 ; i < SN ; i++)
      Serial.println( g.sn[i] );
  Serial.println( '\n' );
  for( int i = 0 ; i < IP_ADDRESS ; i++)
      Serial.println( g.ip_address[i] );
  Serial.println( '\n' );
  for( int i = 0 ; i < IP_ADDRESS ; i++)
      Serial.println( g.ip_address[i] );
  Serial.println( '\n' );
  for( int i = 0 ; i < SERVER_KEY ; i++)
      Serial.println( g.server_public_key[i] );
  Serial.println( '\n' );
  for( int i = 0 ; i < SENSOR_KEY ; i++)
      Serial.println( g.sensor_public_key[i] );
  Serial.println( '\n' );
  for( int i = 0 ; i < CONTROL ; i++)
      Serial.println( g.padding[i] );
  Serial.println( '\n' );
  delay(10000); 
}
