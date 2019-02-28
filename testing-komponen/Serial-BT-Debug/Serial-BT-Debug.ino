/*
 * Template for testing Serial data
 */

// Import Necessary libraries
#include <SoftwareSerial.h>

// Create Software Serial objects
SoftwareSerial Bt(2,4);

// Initialize data
String btData = "";

void setup() {

  // Setup serial for debug
  Serial.begin(9600);

  // Setup serial for bluetooth connection
  Bt.begin(9600);

}

void loop() {

  // Infinite loop on no data
  while ( Bt.available() == 0 ) {
    delay(10);
  }

  char character;

  while ( Bt.available() ) {
    character = Bt.read();
    btData += character ;
    delay(10); //the delay is very vital
  }

  Serial.println(btData.substring(0,5));

  btData = "";
  
}
