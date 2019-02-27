#include<FastLED.h>
#include "SoftwareSerial.h"
#define NUM_LEDS 100
#define LED_PIN0 5

CRGB leds[NUM_LEDS];

int ledPin = 3;

void setup()
{
  // put your setup code here, to run once:

  FastLED.addLeds<WS2812B, LED_PIN0, GRB>(leds, NUM_LEDS);
}

void loop()
{
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 255; i++)
  {
    setColor(i);
    delay(10);
  }

  for (int i = 255; i > 0; i--)
  {
    setColor(i);
    delay(10);
  }
}

void setColor(int val)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(val, val, val);
  }
  FastLED.show();
}
