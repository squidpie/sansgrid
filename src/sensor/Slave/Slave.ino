/* Definitions for communication functions
 * 
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
 
#include <SPI.h> 

//#define SLAVE_SELECT  10        // SS pin 10
//#define SLAVE_READY  9          // Hand shake pin identifying Slave has data to send
//#define LED 13                  // LED for indicating Master recieved data from Slave

char buf [98];
volatile byte pos;
volatile boolean process_it;

void setup (void){
    // Initialize SPI Slave
    spiSlaveInit();
    // Initialize Hand Shake Pin  
    //pinMode( SLAVE_READY , OUTPUT );
    //digitalWrite( SLAVE_READY, HIGH );
}

void loop (void){
    // Assert Slave Ready to send data to Master
    //digitalWrite( SLAVE_READY , LOW);
    //delayMicroseconds(20);
    //digitalWrite( SLAVE_READY , HIGH);
    // Process buffer with data received from Master
    if (process_it){  
        buf [pos] = 0;
        pos = 0;
        process_it = false;
    } 
} 

// Initialize SPI Slave
void spiSlaveInit( void ){
    // have to send on master in, *slave out*
    pinMode( MISO, OUTPUT );
    // turn on SPI in slave mode
    SPCR |= _BV( SPE );
    // get ready for an interrupt
    pos = 0; // buffer empty
    process_it = false;
    // now turn on interrupts
    SPI.attachInterrupt();
}

// SPI interrupt routine
ISR (SPI_STC_vect){ 
    if ( pos < sizeof buf ){
        buf [pos++] = c;  
        if (c == '\n')
            process_it = true;
    }
}
