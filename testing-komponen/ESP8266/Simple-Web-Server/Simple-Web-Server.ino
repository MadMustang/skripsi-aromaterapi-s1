/*
 * A test to make a simple webserver from the WeMos D1
 */

// Import libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Wifi Info
char* ssid = "Ena Komiya";
char* password = "gigabyte";

// Define server object
ESP8266WebServer server;

void setup() {

  // Initialize PC serial
  Serial.begin(115200);

  // LED initial
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Start wifi connection
  WiFi.begin(ssid, password);

  // Wait until connection is established
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting ... ");
    delay(1000);
  }

  // Display message during established connection
  Serial.println("");
  Serial.println("Connection Established");
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Add server route
  server.on("/", [](){server.send(200, "text/plain", "Hello world");});
  server.on("/toggle", toggleLED);

  // Start server
  server.begin();

}

void loop() {

  // Handle requests
  server.handleClient();

}

void toggleLED() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  server.send(204, "");
  Serial.println(millis());
}

