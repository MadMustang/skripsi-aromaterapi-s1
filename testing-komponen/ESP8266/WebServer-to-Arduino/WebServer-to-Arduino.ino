
/*
 * ESP8266 WebServer to control Arduino
 */

// Import libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// Wifi Info
char* ssid = "Ena Komiya";
char* password = "gigabyte";

// Define server object
ESP8266WebServer server;

// LCD Debug
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7);

// Software Serial
SoftwareSerial Arduino(D3, D4);

void setup() {

  // Initialize PC serial
  Serial.begin(9600);

  // LED initial
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Start wifi connection
  connectToWifi();

  // Add server route
  server.on("/", [](){server.send(200, "text/plain", "Hello world");});
  server.on("/toggle", toggleLED);
  server.on("/dfplayer", dfControl);

  // Start server
  server.begin();

  // Initialize Arduino Serial
  Arduino.begin(9600);

}

void loop() {

  // Run request handler if connected to Wi-Fi
  while(WiFi.status() == WL_CONNECTED) {

    // Handle requests
    server.handleClient();
  }

  // Reconnect
  connectToWifi();

}

// ------------------------------ Wi-Fi Handler ------------------------------

// In case if disconnect
void connectToWifi(){

  // Start wifi connection
  WiFi.begin(ssid, password);

  // Wait until connection is established
  while(WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("Connecting ... ");
    delay(1000);
  }

  // Display message during established connection
  //Serial.println("");
  //Serial.println("Connection Established");
  //Serial.println("");
  //Serial.print("IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.println("         ");
}

//--------------------------------- Routes ----------------------------------

// LED toggle
void toggleLED() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  server.send(204, "");
  Serial.println(millis());
}

// DFPlayer
void dfControl() {

  // Get the message
  String command = server.arg("command");
  Serial.println(command);

  // Send to Arduino
  //Arduino.println(command);

  // Wait for response
  //while( Arduino.available() == 0);

  // Catch the response and send to client
  //String resp = serialToString();
  server.send(200, "text/plain", command);
  Serial.println(millis());
  
}

//------------------------------ Helper functions ------------------------------

// Converting Serial data to string
String serialToString(){

  // Initialize local variables
  char character;
  String recData = "";

  // Convert ASCII to String
  while ( Serial.available() > 0 ) {
    character = Serial.read();
    recData += character ;
    delay(10); //the delay is very vital
  }

  // Debug
  //Serial.println(recData);

  return recData;
  
}
