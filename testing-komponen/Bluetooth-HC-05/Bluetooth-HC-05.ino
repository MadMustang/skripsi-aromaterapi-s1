/*
 * Testing using the HC-05 Bluetooth Module
 * Discovered a use for the SoftwareSerial library in order to integrate DF Player
 * Normal UART can be used for debugging
 */
#include <SoftwareSerial.h>// import the serial library

// Set up Software serial object for bluetooth
SoftwareSerial Bt(2,4);

// Initialize BlueTooth data
int btData;
int state = 0;

void setup() {
  // Set up serial and LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize at 0
  digitalWrite(LED_BUILTIN, LOW);

  //Setup Serial
  Bt.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // Enter main loop

  //Read serial
  if(Bt.available()){ // Checks whether data is comming from the serial port
    btData = Bt.read();
  }

  //On or Off
  if (btData == '0') {
    digitalWrite(LED_BUILTIN, LOW); // Turn LED OFF

    if ( state == 0 ) {
      Bt.println("LED: ON"); // Send back, to the phone, the String "LED: ON"
      Serial.println("LED OFF");
      state = 1;
    }
  
  } else if (btData == '1') {
    digitalWrite(LED_BUILTIN, HIGH);

    if ( state == 1 ) {
      Bt.println("LED: OFF");
      Serial.println("LED ON");
      state = 0;
    }
    
  } 

  //Safe delay
  delay(100);
}
