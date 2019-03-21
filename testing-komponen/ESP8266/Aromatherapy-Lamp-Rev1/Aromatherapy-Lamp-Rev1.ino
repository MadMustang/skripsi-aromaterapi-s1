
/*
 * Multifunctional Aromatherapy Rev 0.1
 * 
 * This will be used to incorporate all of the systems that has already been tested.
 * It will use MQTT and make sure that all the systems work normally.
 * 
 */

//----------------------------- Libraries ------------------------------------

// Library import
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

//-------------------------Global Variables ---------------------------------

// MQTT Topics - Subscribe
char* setLEDColorTopic = "led/color";
char* setLEDBrightTopic = "led/brightness";
char* relaySwitchTopic = "switch";
char* dfPlayTrackTopic = "dfplayer/playtrack";
char* dfPlayPauseTopic = "dfplayer/toggle";
char* dfVolume = "dfplayer/volume";

// MQTT Topics - Publish
char* waterLevelTopic = "Sensor";
char* setLEDBrightStatTopic = "led/brightness/status";
char* dfplayerStatusTopic = "dfplayer/status";

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
unsigned long now;
unsigned long last = 0;

// Color LED Variables
#define PIN D3
#define LED_COUNT 4
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);
int ledBrightness = 255;
int r = 0;
int g = 0;
int b = 0;

// Water level variables
#define trigPin D6
#define echoPin D7
long echoDuration;
double cmHeight;
int levelPercent;
double fullHeight = 4;
double emptyHeight = 8;

// DFPlayer Variables
#define Start_Byte 0x7E // Inisial DFPlayer
#define Version_Byte 0xFF // Inisial DFPlayer
#define Command_Length 0x06 // Inisial DFPlayer
#define End_Byte 0xEF // Inisial DFPlayer
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info] // Inisial DFPlayer
#define ACTIVATED LOW
#define statusPin D0
#define DF_TX D1
#define DF_RX D2
SoftwareSerial DFPlayer(DF_RX, DF_TX);

// Humidifier Relay
#define HUM_PIN D5

// Delay variables (in seconds)
unsigned long waterLevelDelay = 5;

//------------------------------- Main Program ---------------------------------

void setup() {

  // DF player
  Serial.begin(9600);
  DFPlayer.begin(9600);
  delay(100);
  dfInit();

  // LED initial
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Relay initial
  pinMode(HUM_PIN, OUTPUT);
  digitalWrite(HUM_PIN, HIGH); //For off

  // Initiate water level
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  // Color LED initialization
  strip.begin();
  delay(50);
  setLEDColor(255, 255, 255); // set to white
  //setLEDBrightness(255); // maximum brightness

  // DFPlayer status pin
  pinMode(statusPin, INPUT);

  // Start wifi connection
  connectToWifi();

  // MQTT Connect
  connectAsClient();

}

void loop() {

  // Run request handler if disconnected from Wi-Fi
  while(WiFi.status() != WL_CONNECTED) {

    // Reconnect
    connectToWifi();

    // Reestablish client
    connectAsClient();
    
  }

  // Handle requests
  client.loop();

  // Water Level
  now = millis();
  if (now - last > (waterLevelDelay * 1000)) {
    waterLevel();
    last = millis();
  }

}

//------------------------------- MQTT ------------------------------------

// Connect as client
void connectAsClient() {

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {

    // Connect debug
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
  //client.publish(setLEDBrightStatTopic, String(ledBrightness).c_str() );

  // Subscriptions
  //client.subscribe("test/esp8266");// here is where you later add a wildcard
  client.subscribe(relaySwitchTopic);
  client.subscribe(setLEDColorTopic);
  client.subscribe(setLEDBrightTopic);
  client.subscribe(dfPlayTrackTopic);
  client.subscribe(dfPlayPauseTopic);
  client.subscribe(dfVolume);

  // Debug
  Serial.println("Subcribed to topics");
  
}

//----------------------- MQTT Callback (the other main) -------------------------

// Calback function
void callback(char* topic, byte* payload, unsigned int length) {

  // Topic debug
  Serial.print("Messageved in topic: ");
  Serial.println(topic);
  Serial.println();

  // Payload processing
  char ch;
  String mess = "";
  for (unsigned int i = 0; i < length; i++) {
    ch = (char) payload[i];
    mess += ch;
  }

  String top = topic;

  // Simple LED
  if (top == relaySwitchTopic) {  // Relay switch
    ledToggle(mess);
  } else if (top == setLEDColorTopic) { // LED strip color

    // Get index of string to seperate data
    int redIndex = mess.indexOf("|");
    int greenIndex = mess.indexOf("|", redIndex + 1);
    int blueIndex = mess.indexOf("|", greenIndex + 1);

    // Send to Set Led color function
    setLEDColor(mess.substring(0, redIndex).toInt(), mess.substring(redIndex+1, greenIndex).toInt(), mess.substring(greenIndex+1, blueIndex).toInt());
    
  } else if (top == setLEDBrightTopic) { // LED Strip brightness

    // Adjust
    setLEDBrightness(mess.toInt());
    
  } else if (top == dfPlayTrackTopic) { // DF player play specific track

    //chooseTrack(mess.toInt());
    playBackMode(mess.toInt());
    
  } else if (top == dfPlayPauseTopic) { // Toggle pause or play

    // Toggle pause or play
    dfPlayerToggle();
    
  } else if(top == dfVolume) {

    // Adjust volume
    setVolume(mess.toInt());
    
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

//------------------------------- Simple LED ------------------------------

void ledToggle(String toggle) {

  // Validate
  if (toggle == "off"){
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(HUM_PIN, HIGH);
    } else if (toggle == "on") {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(HUM_PIN, LOW);
    }
  
}

//------------------------------ Water Level ------------------------------

void waterLevel() {

  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  echoDuration = pulseIn(echoPin, HIGH);

   // Convert the time into a distance
  cmHeight = (echoDuration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  
  // Water level percentage
  levelPercent = (emptyHeight - cmHeight)/fullHeight * 100;
  if (levelPercent >= 100) levelPercent = 100;
  if (levelPercent <= 0) levelPercent = 0;

  // Publish to broker
  client.publish(waterLevelTopic, String(levelPercent).c_str() );
  delay(10);
  client.publish("water/debug", String(cmHeight).c_str() );
  
}

//--------------------------- Color LED Strip ------------------------------

// Set LED Color
void setLEDColor(int red, int green, int blue) {

  // Set colors
  while ( r != red || g != green || b != blue ) {
    if ( r < red ) r += 1;
    if ( r > red ) r -= 1;

    if ( g < green ) g += 1;
    if ( g > green ) g -= 1;

    if ( b < blue ) b += 1;
    if ( b > blue ) b -= 1;

    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(r, g, b));
    }

    // Show
    strip.show();
  
    delay(4);
  }

  // Adjust to current values
  r = red;
  g = green;
  b = blue;

  
  delay(100);
  
}

// Set LED brightness
void setLEDBrightness(int num) {

  // Set brightness
  if (num > ledBrightness) { // if current brightness is less than requested

    for (int i = ledBrightness; i < num+1; i++ ) {
      strip.setBrightness(i);
      strip.show();
      delay(4);
    }
    
  } else if (num < ledBrightness) {

     for (int i = ledBrightness; i > num-1; i-- ) {
      strip.setBrightness(i);
      strip.show();
      delay(4);
    }

    // Make sure color is the same
    setLEDColor(r,g,b);
    
  }

  // Set ledBrightness to current
  ledBrightness = num;

  // Publish brightness data
  client.publish(setLEDBrightStatTopic, String(ledBrightness).c_str() );
  delay(100);
}

//------------------------------------ DF Player ----------------------------------------------

void playFirst() // void untuk memutar lagu pertama
{
  execute_CMD(0x3F, 0, 0);
  delay(1000);
  setVolume(30);
  delay(1000);
  execute_CMD(0x11, 0, 1);
  delay(1000);
}

void dfInit() {
  execute_CMD(0x3F, 0, 0);
  delay(1000);
}

void pause() // void untuk melakukan pause
{
  execute_CMD(0x0E, 0, 0);
  delay(1000);
}

void play() // void untuk melakukan play
{
  execute_CMD(0x0D, 0, 1);
  delay(1000);
}

void playNext() // void untuk melakukan play Next
{
  execute_CMD(0x01, 0, 1);
  delay(1000);
}

void playPrevious() // void untuk melakukan PlayPrevious
{
  execute_CMD(0x02, 0, 1);
  delay(1000);
}

void setVolume(int volume) // Void untuk mengatur Volume
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void chooseTrack(int num) {
  execute_CMD(0x03, 0, num);
  delay(1000);
}

void playBackMode(int num) {
  execute_CMD(0x08, 0, num);
  delay(1000);
}

void repeatPlay(int num) {
  execute_CMD(0x11, 0, num);
  delay(1000);
}

// Used a status to determine if DF player is playing anything
boolean dfPlayerIsPlaying() {

  // Simple if else
  if (digitalRead(statusPin) == LOW)
    return true;
  else
    return false;
  
}

// DFPlayer toggle
void dfPlayerToggle() {
  if (dfPlayerIsPlaying())
    pause();
  else
    play();
}

void execute_CMD(byte CMD, byte Par1, byte Par2) // Inisial DFPlayer
// Excecute the command and parameters
{
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                            Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte
                          };
  //Send the command line to the module
  for (byte k = 0; k < 10; k++)
  {
    DFPlayer.write( Command_line[k]);
  }
}
