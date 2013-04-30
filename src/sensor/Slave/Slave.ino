/* SPI Slave Test Code
 * Designed to run on the Arduino UNO
 */
 
#include <SPI.h>

#define SLAVE_READY 7
#define NUM_BYTES 12

char rx [ NUM_BYTES + 1 ];
//char tx [ NUM_BYTES + 1 ];
volatile byte pos;
// volatile byte pos2;
volatile boolean process_it;

void setup (void){
    // Initialize Serial bps
    //Serial.begin (115200);

    // Initialize Master In Slave Out Port
    pinMode(MISO, OUTPUT);
  
    // Initialize Slave Ready port
    //pinMode(SLAVE_READY, OUTPUT);
    //digitalWrite(SLAVE_READY, HIGH);
  
    // Initialize SPI in slave mode
    SPCR |= _BV(SPE);
  
    // Set counters to zero SPI interrupt 
    pos = 0;   // rxfer empty
    //pos2 = 0;  // txfer empty
    process_it = false;

    // Initialize SPI interrupt
    SPI.attachInterrupt();
}

//ISR (/*XBEE Interrupt pin (2 or 3?)*/){
    // Interrupt Code
//)

// SPI interrupt routine
ISR (SPI_STC_vect){
    // Read byte sent from Master SPI  
    byte c = SPDR;  
    
    // Send byte
    /*if ( digitalRead( SLAVE_READY ) == LOW ){
        if ( pos2 < ( NUM_BYTES + 1 )){
            tx[ pos2++ ] = c;
            // example: newline means time to process rxfer
            if ( pos2 == NUM_BYTES )               
                process_it = true;  
        }
    }*/
    
    // Store byte sent from Master SPI into buffer
    if ( pos < ( NUM_BYTES  )){
        rx[ pos++ ] = c;
        // example: newline means time to process rxfer
        if ( pos == NUM_BYTES - 1 )
            process_it = true;
    }  
}  

void loop (void){
    if ( process_it ){
        //tx [pos] = 0;  
        rx [pos] = 0;  
        //Serial.println (rx);
        pos = 0;
        //pos2 = 0;
        process_it = false;
    }
}

