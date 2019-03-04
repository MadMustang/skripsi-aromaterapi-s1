/*
 * Trying to make an app to integrate so that in can control the DF player through Bluetooth
 * First step: Make an app that can send text data to the Arduino and view it with the Serial Monitor
 * Second Step: Control LED WS1218B with App
 * Third step: Control DF Player with app
 */

// Import Necessary libraries
#include <SoftwareSerial.h>
#include <FastLED.h>

// DFPlayer Variables
#define Start_Byte 0x7E // Inisial DFPlayer
#define Version_Byte 0xFF // Inisial DFPlayer
#define Command_Length 0x06 // Inisial DFPlayer
#define End_Byte 0xEF // Inisial DFPlayer
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info] // Inisial DFPlayer
#define ACTIVATED LOW

//LED Variables
#define DATA_PIN 6        //LED Data PIN
#define NUM_LEDS 1      //Number of leds on the matrix
CRGB leds[NUM_LEDS];

// Create Software Serial objects
SoftwareSerial Bt(2,4);
SoftwareSerial DFPlayer(11,10);

// Initialize data
String btData = "";
boolean isPlaying = true;
int led = 3;
int rly = 7;

void setup() {

  pinMode(rly, OUTPUT);
  digitalWrite(rly, HIGH);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  // Setup serial for debug
  Serial.begin(9600);

  // Setup DFPlayer
  DFPlayer.begin(9600);
  delay(10);

  // Setup serial for bluetooth connection
  Bt.begin(9600);

  // Init LED
  FastLED.addLeds<WS2812B, DATA_PIN, GRB> (leds, NUM_LEDS);

  /*
   * Somewhat, the bluetooth should be initialized after the DFPlayer
   */
  delay(10);
  playFirst();
  setColor(255);
}

void loop() {

  // Infinite loop on no data
  while ( Bt.available() == 0 ) {
    delay(10);
  }

  // Insert data to data buffer
  btData = btToString();

  if ( btData.substring(0,5) == "Hello") {
    Serial.println("Playing next ... ");
    playNext();
  } else if ( btData.substring(0,5) == "There") {

    if (isPlaying) {
      Serial.println("Pause ... ");
      setColor(0);
      //pause();
      isPlaying = false;
      
      digitalWrite(rly, LOW); 
      delay(10);
      digitalWrite(led,LOW);
           
    } else {
      Serial.println("Play ... ");
      setColor(255);
      //play();
      isPlaying = true;
      
      digitalWrite(rly, HIGH); 
      delay(10);
      digitalWrite(led,HIGH);
    }
    
  }

  btData = "";
  
}

//------------------------------------ String Manipulation ---------------------------------------------

// Converting Bluetooth data to string
String btToString(){

  // Initialize local variables
  char character;
  String recData = "";

  // Convert ASCII to String
  while ( Bt.available() > 0 ) {
    character = Bt.read();
    recData += character ;
    delay(10); //the delay is very vital
  }

  // Debug
  //Serial.println(recData);

  return recData;
  
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

//-------------------------------------------- LED Functions ----------------------------------------------

void setColor(int val)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(val, val, val);
  }
  FastLED.show();
}