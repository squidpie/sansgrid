To use libraries in source code, copy folders into "/Documents/Arduino/libraries/".
Then either paste this in your sketch at the top:

     #include <sensorPayloadHandler.h>
     #include <sgSerial.h>
     #include <sensorPayloads.h>
     #include <sensorParse.h>
     #include <SPI.h>

or select SKETCH then IMPORT LIBRARIES then select the libraries to include.


For more detail and information on installing libraries, see: 



          http://arduino.cc/en/Guide/Libraries



To compile on Uno ensure to comment out: //#define DUE 1
in the sgSerial.cpp file.

To compile on DUE ensure to NOT comment out:  #define DUE 1
in the sgSerial.cpp file.

To implement a push button mate ensure to not comment out: #define PUSH_BUTTON 1
in the sensorPayloadHandler.cpp.
