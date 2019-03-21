
/*
 * ESP8266 WebServer to control Arduino
 */

// Import libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

// DFPlayer Variables
#define Start_Byte 0x7E // Inisial DFPlayer
#define Version_Byte 0xFF // Inisial DFPlayer
#define Command_Length 0x06 // Inisial DFPlayer
#define End_Byte 0xEF // Inisial DFPlayer
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info] // Inisial DFPlayer
#define ACTIVATED LOW

// LED Variables
#define PIN D3
#define LED_COUNT 4

// Wifi Info
char* ssid = "Ena Komiya";
char* password = "gigabyte";
String myHostname = "Aromaterapi";

// Data
boolean isPlaying = true;

// Define server object
ESP8266WebServer server;

// LCD Debug
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7);

// LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

void setup() {

  // Initialize PC serial
  Serial.begin(9600);

  // LED initial
  pinMode(LED_BUILTIN, OUTPUT);
  delay(50);
  strip.begin();

  //LCD
  // activate LCD module
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  
  // Start wifi connection
  connectToWifi();

  // Add server route
  server.on("/", [](){server.send(200, "text/plain", "Hello world");});
  server.on("/toggle", toggleLED);
  server.on("/dfplayer", dfControl);
  server.on("/playtrack", trackPick);
  server.on("/setcolor", setColor);

  // Start server
  server.begin();

  lcdPrint("Rdy");
  
  playFirst();
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

  // Set hostname
  WiFi.hostname(myHostname);

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
  lcd.print(WiFi.localIP());
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
  lcdPrint(command);

  if ( command.substring(0,5) == "Hello") {
    Serial.println("Playing next ... ");
    playNext();
  } else if ( command.substring(0,5) == "There") {

    if (isPlaying) {
      lcdPrint("Pause ... ");
      pause();
      isPlaying = false;
    } else {
      lcdPrint("Play ... ");
      play();
      isPlaying = true;
    }
    
  }
  
  server.send(200, "text/plain", command);
  
}

// Track pick
void trackPick() {

  // Get the message
  String track = server.arg("track");
  lcdPrint("Track " + track);

  // Play track
  chooseTrack(track.toInt());

  // Calback
  server.send(200, "text/plain", "Playing");
}

// LED Color
void setColor() {
  String r = server.arg("r");
  String g = server.arg("g");
  String b = server.arg("b");

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r.toInt(), g.toInt(), b.toInt()));
    //strip.setPixelColor(i, strip.Color(25, 155, 23));
    //delay(20);
    //strip.show();
  }
  strip.show();

  // Calback
  server.send(200, "text/plain", r+" "+g+" "+b);
}

//------------------------------------------- LCD ------------------------------


void lcdPrint(String text){
  lcd.home();
  lcd.print("               ");
  delay(10);
  lcd.home();
  lcd.print(text);
  lcd.setCursor(0,1);
  lcd.print(millis());
}

//------------------------------------ DF Player Functions ----------------------------------------------

void playFirst() // void untuk memutar lagu pertama
{
  execute_CMD(0x3F, 0, 0);
  delay(1000);
  setVolume(30);
  delay(1000);
  execute_CMD(0x11, 0, 1);
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
    Serial.write( Command_line[k]);
  }
}
