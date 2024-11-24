#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 1
#define LED_PIN 47

CRGB leds[NUM_LEDS];

int brightness = 10;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
}

void loop()
{

    leds[0] = CRGB(255, 0, 0);
    FastLED.show();
    delay(500);
    leds[0] = CRGB(0, 255, 0);
    FastLED.show();
    delay(500);
    leds[0] = CRGB(0, 0, 255);
    FastLED.show();
    delay(500);
}