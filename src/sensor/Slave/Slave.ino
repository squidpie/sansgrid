/* SPI Slave Test Code
 * Designed to run on the Arduino UNO
 */
 
#include <SPI.h>

#define SLAVE_READY 7
#define NUM_BYTES 98
byte command;
byte rx [ NUM_BYTES ];
// Fly
/*byte tx [ NUM_BYTES ] = { 0xF0,// Control Byte 1 BYTE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01, // IP Address 16 BYTES
        0x10, // Payload (Data Type) 1 BYTE
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
        0x00,0x00,0x53,0x61,0x6E,0x73,0x67,0x72,0x69,0x64 }; // Payload 
        // Network ID 80 BYTES
        // Payload 81 BYTES
// Peck
/*byte tx [ NUM_BYTES ] = { 0xAD,// Control Byte 1 BYTE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01, // IP Address 16 BYTES
        0x01, // Payload (Data Type) 1 BYTE
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01,// Router IP 16 BYTES
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x0A, // Assigned IP 16 BYTES
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01, // Server ID 16 BYTES
        0x01, // Recognition 1 BYTE
        0x00,0x00,0x00,0x00, // Manid 4 BYTES
        0x00,0x00,0x00,0x00, // Modnum 4 BYTES
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // SN 8 BYTES
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00 }; // Unused 15 BYTES
        // Payload 81 BYTES
// Sing
byte tx [ NUM_BYTES ] = { 0xAD,// Control Byte 1 BYTE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01, // IP Address 16 BYTES
        0x02, // Payload (Data Type) 1 BYTE
	0x09,0xC4,0x59,0xDC,0xA6,0xF5,0xF0,0x16,0x54,0xE5,0xC0,0x87,0xB0,0x94,
        0xBA,0x89,0x16,0xFF,0x0C,0x26,0xEF,0xE9,0xAF,0x53,0x02,0x83,0x7D,0x1B,
        0xF9,0x6A,0x94,0x61,0x57,0x27,0xDF,0xDE,0xB7,0x0B,0x2D,0xA4,0xC6,0x23,
        0x8D,0x66,0x15,0x38,0x4A,0x28,0xE3,0x3F,0x29,0xB1,0x2F,0xBD,0xB4,0xED,
        0x28,0xF1,0x3D,0xED,0x25,0xB3,0xC1,0xE2, // Payload Server Public Key 
        // 64 BYTES
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00 }; // Unused 16 BYTES
        // Payload 81 Bytes
// Nest
byte tx [ NUM_BYTES ] = { 0xAD,// Control Byte 1 BYTE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xA8,
        0x00,0x01, // IP Address 16 BYTES
        0x10, // Payload (Data Type) 1 BYTE
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0 }; // Payload Unused
        // 80 BYTES
        // Payload 81 BYTES
*/
volatile byte pos;
//volatile byte pos2;
boolean process_it;

void setup (void){
    // Initialize Serial bps
    Serial.begin (9600);

    // Initialize Master In Slave Out Port
    pinMode(MISO, OUTPUT);
  
    // Initialize Slave Ready port
    pinMode(SLAVE_READY, OUTPUT);
    digitalWrite(SLAVE_READY, HIGH);
  
    // Initialize SPI in Slave mode
    SPCR |= _BV(SPE);
  
    // Set buffer Counters to Zero 
    pos = 0;   // rxfer empty
    //pos2 = 0;  // txfer empty
    command = (uint8_t) 0x00;
    
    // Set Process flag to default
    process_it = false;

    // Initialize SPI interrupt
    SPI.attachInterrupt();
}

void loop (void){
    //delay(5000);
    // Assert Slave Ready Low, transfer buffer ready to send
    //if ( command == 0x00 )
    //digitalWrite(SLAVE_READY , LOW );
    // Delay to allow Master to process interrupt
    //delayMicroseconds(60);
    // Assert Slave Ready back to High
    //digitalWrite(SLAVE_READY , HIGH );
    //delay(5000);
    // Buffer is Full, process SPI Packet
    if ( process_it ){
        Serial.println( "Processing" );
        //Serial.println( pos );
        for( int i = 0 ; i < NUM_BYTES ; i++ )
            Serial.println( rx[i] );
        Serial.println( "Packet End");
        // Reset Command from Control Byte
        command = (uint8_t) 0x00;
        // Reset Buffer Position to Zero
        pos = 0;
        //pos2 = 0;
        // Reset Process SPI Packet Flag
        process_it = false;
    }
    //while(1){};
}

// SPI interrupt routine
ISR (SPI_STC_vect){
    // Read byte sent from Master SPI  
    byte c = SPDR; 
    
    // Command to store SPI data either in
    // Transfer or Receive Buffer
    switch (command){
    case 0x00:
        digitalWrite(SLAVE_READY , HIGH );
        // Store initial Command
        command = (uint8_t) c; 
        SPDR = 0;
        break;
    case 0xAD:
        // Store byte read from Master in receive buffer
        if ( pos < sizeof rx ){
            rx[ pos++ ] = c;
            // If buffer is full process it
            if ( pos == NUM_BYTES - 1 )
                process_it = true;
        }
        break;
    case 0xFD:
        // Send Slave Data in transfer buffer to Master 
        //SPDR=tx[ pos2++ ];
        // If buffer is full process it
        //if ( pos2 == NUM_BYTES - 1  )               
            //process_it = true; 
        break;
    default:
        break;
    }
}  

