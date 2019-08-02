

/*
   Multifunctional Aromatherapy Rev 0.1

   This will be used to incorporate all of the systems that has already been tested.
   It will use MQTT and make sure that all the systems work normally.

*/

//----------------------------- Libraries ------------------------------------

// Library import
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <Math.h>
#include <MedianFilterLib.h>

//-------------------------Global Variables ---------------------------------

// MQTT Topics - Subscribe
char* setLEDColorTopic = "led/color";
char* ledSwitchTopic = "led/switch";
char* setLEDBrightTopic = "led/brightness";
char* relaySwitchTopic = "humidifier/switch";
char* dfPlayTrackTopic = "dfplayer/playtrack";
char* dfPlayPauseTopic = "dfplayer/toggle";
char* dfVolume = "dfplayer/volume";

// MQTT Topics - Publish
char* waterLevelTopic = "Sensor";
char* setLEDBrightStatTopic = "led/brightnesstatus";
char* setLEDColorStatTopic = "led/colorstatus";
char* dfplayerStatusTopic = "dfplayer/status";
char* relayStatusTopic = "humidifier/status";

// Wifi Info
char* ssid = "Ena Komiya";
char* password = "gigabyte";
#define MODE_PIN D4

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
#define LED_COUNT 50
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
int lastLevelPercent = 0;
double fullHeight = 3.5;
double emptyHeight = 8.5; // either 9.6 or 8.5

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
boolean firstPlay = true;
int normalVolumeLevel = 20;
int currentVolume = normalVolumeLevel;

// Humidifier Relay
#define HUM_PIN D5

// Delay variables (in miliseconds)
unsigned long timedDelay = 5000;

//----------------------------- Relay Switch ------------------------------

void relayToggle(String toggle) {

  // Validate
  if (toggle == "off") {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(HUM_PIN, HIGH);
    client.publish(relayStatusTopic, "Off");
  } else if (toggle == "on") {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(HUM_PIN, LOW);
    client.publish(relayStatusTopic, "On");
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
    if (client.connect("ESP8266Client", mqttUser, mqttPassword, "DC", 0, false, "Device Disconnected"))
      Serial.println("Connected");
    else {
      Serial.print("Failed state code: ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  // First publish
  client.publish("test/esp8266", "Hello ESP World");
  //client.publish(setLEDBrightStatTopic, String(ledBrightness).c_str() );

  // Subscriptions
  //client.subscribe("test/esp8266");// here is where you later add a wildcard
  client.subscribe(relaySwitchTopic);
  client.subscribe(setLEDColorTopic);
  client.subscribe(ledSwitchTopic);
  client.subscribe(setLEDBrightTopic);
  client.subscribe(dfPlayTrackTopic);
  client.subscribe(dfPlayPauseTopic);
  client.subscribe(dfVolume);

  // Debug
  Serial.println("Subcribed to topics");

}

//------------------------------- Wi-Fi -----------------------------------

// Connect to Wi-Fi
void connectToWifi() {

  // Start wifi connection
  WiFi.begin(ssid, password);
  wifi_station_set_hostname("AromatherapyLamp");

  // Wait until connection is established
  while (WiFi.status() != WL_CONNECTED)
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

// Check for Setup or Operation Mode
boolean modeSelect() {



}

//------------------------------ Water Level ------------------------------

void waterLevel() {

  double dataUltra[5];
  MedianFilter<double> medianFilter(5);
  
  for (int i = 0; i < 5; i++) {
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
    double echo = pulseIn(echoPin, HIGH);

    // Debug Echo Duration
    Serial.print("T (microseconds): ");
    Serial.println(echo);

    medianFilter.AddValue(echo);
    delay(20);
  }

  echoDuration = medianFilter.GetFiltered();
  
  // Convert the time into a distance
  //cmHeight = (echoDuration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  double cmDiagonal = 0.017 * echoDuration;
  cmHeight = sqrt((cmDiagonal * cmDiagonal) - 2.25);
  //cmHeight = cmDiagonal;

  // Water level percentage
  //levelPercent = (emptyHeight - cmHeight)/fullHeight * 100;
  levelPercent = mapPercent(cmHeight, emptyHeight, fullHeight, 0, 100);
  if (levelPercent >= 100) levelPercent = 100;
  if (levelPercent <= 0) levelPercent = 0;

  // Publish to broker
  //  if (levelPercent != lastLevelPercent) {
  //    client.publish(waterLevelTopic, String(levelPercent).c_str() );
  //    delay(10);
  //    client.publish("water/debug", String(cmHeight).c_str() );
  //    lastLevelPercent = levelPercent;
  //  }

  // Debug Text
  String debugSensor = String(levelPercent) + "|" + String(cmHeight) + "|" + String(echoDuration);

  client.publish(waterLevelTopic, String(levelPercent).c_str() );
  delay(10);
  client.publish("water/debug", String(debugSensor).c_str() );
  lastLevelPercent = levelPercent;


}

// Mapping function
int mapPercent(double x, double in_min, double in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
  String feedback = String(r) + "|" + String(g) + "|" + String(b);


  delay(100);

  // Send feedback data
  client.publish(setLEDColorStatTopic, String(feedback).c_str());

}

// Set LED brightness
void setLEDBrightness(int num) {

  // Set brightness
  if (num > ledBrightness) { // if current brightness is less than requested

    for (int i = ledBrightness; i < num + 1; i++ ) {
      strip.setBrightness(i);
      strip.show();
      delay(4);
    }

  } else if (num < ledBrightness) {

    for (int i = ledBrightness; i > num - 1; i-- ) {
      strip.setBrightness(i);
      strip.show();
      delay(4);
    }

    // Make sure color is the same
    setLEDColor(r, g, b);

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
  setVolume(normalVolumeLevel);
  delay(1000);
  currentVolume = normalVolumeLevel;
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
  delay(100);
}

void play() // void untuk melakukan play
{
  execute_CMD(0x0D, 0, 1);
  delay(100);
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
  delay(300);
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

  return digitalRead(statusPin) == LOW;

}

// DFPlayer toggle
void dfPlayerToggle() {

  int vol;

  if (dfPlayerIsPlaying()) {

    vol = currentVolume;

    while (vol > 5) {
      setVolume(vol);
      vol = vol - 2;
    }

    pause();

  } else {

    play();

    vol = 5;

    while (vol <= currentVolume) {

      vol = vol + 2;
      if (vol >= currentVolume)
        setVolume(currentVolume);
      else
        setVolume(vol);

    }
  }
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
    relayToggle(mess);
  } else if (top == setLEDColorTopic) { // LED strip color

    // Get index of string to seperate data
    int redIndex = mess.indexOf("|");
    int greenIndex = mess.indexOf("|", redIndex + 1);
    int blueIndex = mess.indexOf("|", greenIndex + 1);

    // Send to Set Led color function
    setLEDColor(mess.substring(0, redIndex).toInt(), mess.substring(redIndex + 1, greenIndex).toInt(), mess.substring(greenIndex + 1, blueIndex).toInt());

  } else if (top == setLEDBrightTopic) { // LED Strip brightness

    // Pre-process
    int brightness = mess.toInt();
    if (brightness < 30) brightness = 30;

    // Adjust
    setLEDBrightness(brightness);

  } else if (top == dfPlayTrackTopic) { // DF player play specific track

    // Check if playing
    if (dfPlayerIsPlaying()) {
      int vol = currentVolume;

      while (vol > 5) {
        setVolume(vol);
        vol = vol - 2;
      }

      // Set current volume
      currentVolume = normalVolumeLevel;
      
      pause();
    }

    //chooseTrack(mess.toInt());
    setVolume(normalVolumeLevel);
    firstPlay = false;
    playBackMode(mess.toInt());

  } else if (top == dfPlayPauseTopic) { // Toggle pause or play

    if (!firstPlay) {

      // Toggle pause or play
      dfPlayerToggle();

    }

  } else if (top == dfVolume) {

    // Adjust volume
    int volNow = mess.toInt();
    int volTemp = currentVolume;

    if (currentVolume < volNow) {

      while (volTemp <= volNow) {

        volTemp = volTemp + 2;
        if (volTemp >= volNow)
          setVolume(volNow);
        else
          setVolume(volTemp);
      }

    } else if (currentVolume > volNow) {

      while (volTemp >= volNow) {

        volTemp = volTemp - 2;
        if (volTemp <= volNow)
          setVolume(volNow);
        else
          setVolume(volTemp);
      }

    }


    currentVolume = volNow;

  }
}


  //------------------------------- Main Program ---------------------------------

  void setup() {

    // DF player
    Serial.begin(9600);
    DFPlayer.begin(9600);
    delay(100);
    dfInit();

    // Mode Select


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
    while (WiFi.status() != WL_CONNECTED) {

      // Reconnect
      connectToWifi();

      // Reestablish client
      connectAsClient();

    }

    // Handle requests
    client.loop();

    // Timed operations
    now = millis();
    if (now - last > timedDelay) {

      // Debug MQTT connection
      Serial.print("Client connected status: ");
      Serial.println(client.connected());

      // Validate connection status
      if (!client.connected())
        connectAsClient();

      // Water Level
      waterLevel();

      // Reset timer
      last = millis();

    }

  }
