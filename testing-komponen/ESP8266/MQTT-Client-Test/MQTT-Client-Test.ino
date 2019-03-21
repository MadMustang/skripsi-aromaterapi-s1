/*
 * Simple MQTT Client
 */

// Library import
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wifi Info
char* ssid = "Ena Komiya";
char* password = "gigabyte";

// MQTT Broker
const char* mqttServer = "m16.cloudmqtt.com";
const int mqttPort = 14311;
const char* mqttUser = "xmxzgirv";
const char* mqttPassword = "VIlzm7f9S8So";

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Timer interrupt
int gauge = 0;
unsigned long now;
unsigned long last = 0;

void setup() {

  // Serial debug
  Serial.begin(115200);

  // LED initial
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Start wifi connection
  connectToWifi();

  // MQTT Connect
  connectAsClient();

}

void loop() {
  
  // Run request handler if connected to Wi-Fi
  while(WiFi.status() != WL_CONNECTED) {

    //Reconnect
    connectToWifi();
    
  }

  // Handle requests
  client.loop();

  now = millis();
  if (now - last > 3000 ) {
    last = millis();

    spamming();
  }
}

//------------------------------- Wi-Fi -----------------------------------

// In case if disconnect
void connectToWifi(){

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
}

//------------------------------- MQTT ------------------------------------

// Connect as client
void connectAsClient() {

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    // Validate
    if(client.connect("ESP8266Client", mqttUser, mqttPassword ))
      Serial.println("connected");
    else {
      Serial.print("failed state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // First publish
  client.publish("test/esp8266","Hello ESP World");
  client.subscribe("test/esp8266");// here is where you later add a wildcard
  client.subscribe("switch");
}

// Calback function
void callback(char* topic, byte* payload, unsigned int length) {

  // Process message
  Serial.print("Messageved in topic: ");
  Serial.println(topic);
  Serial.println();

  // Payload 
  char ch;
  String mess = "";
  for (unsigned int i = 0; i < length; i++) {
    ch = (char) payload[i];
    mess += ch;
  }
  Serial.print("Payload: ");
  Serial.println(mess);

  String top = topic;
  
  // LED
  if (top == "switch") {
    Serial.println("It is a switch function");
    ledToggle(mess);
  }
}

//--------------------------------- Timer --------------------------------------------

// Spamming
void spamming() {
  
  // Send gauge
  client.publish("Sensor", String(gauge).c_str() );
  Serial.println(gauge);

  // Increment
  gauge++;
  if (gauge > 100)
    gauge = 0;
  
}

//--------------------------------- LED ---------------------------------------------

void ledToggle(String toggle) {

  // Validate
  if (toggle == "off"){
      digitalWrite(LED_BUILTIN, HIGH);
    } else if (toggle == "on") {
      digitalWrite(LED_BUILTIN, LOW);
    }
  
}

