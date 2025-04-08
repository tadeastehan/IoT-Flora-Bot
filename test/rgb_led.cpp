#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 1
#define LED_PIN 47

CRGB leds[NUM_LEDS];

int brightness = 10;
int maxBrightness = 255;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

    // Show 5 different brightness levels with white color
    for (int i = 0; i < 5; i++)
    {
        int currentBrightness = map(i, 0, 4, 10, maxBrightness);
        FastLED.setBrightness(currentBrightness);
        leds[0] = CRGB(255, 255, 255); // White color
        FastLED.show();
        Serial.print("Brightness level: ");
        Serial.println(currentBrightness);
        delay(1000); // Wait 1 second between brightness levels
    }

    // Set to maximum brightness for the rainbow effect
    FastLED.setBrightness(maxBrightness);
}

void loop()
{
    // Cycle through all possible RGB combinations
    static uint8_t r = 0, g = 0, b = 0;
    static uint8_t colorState = 0;

    // Set the LED to current RGB values
    leds[0] = CRGB(r, g, b);
    FastLED.show();

    // State machine to cycle through all colors
    // State 0: R increases, G=0, B=0
    // State 1: R=255, G increases, B=0
    // State 2: R decreases, G=255, B=0
    // State 3: R=0, G=255, B increases
    // State 4: R=0, G decreases, B=255
    // State 5: R increases, G=0, B=255
    // State 6: R=255, G=0, B decreases
    switch (colorState)
    {
    case 0: // Increase R
        r++;
        if (r == 255)
            colorState = 1;
        break;
    case 1: // Increase G
        g++;
        if (g == 255)
            colorState = 2;
        break;
    case 2: // Decrease R
        r--;
        if (r == 0)
            colorState = 3;
        break;
    case 3: // Increase B
        b++;
        if (b == 255)
            colorState = 4;
        break;
    case 4: // Decrease G
        g--;
        if (g == 0)
            colorState = 5;
        break;
    case 5: // Increase R
        r++;
        if (r == 255)
            colorState = 6;
        break;
    case 6: // Decrease B
        b--;
        if (b == 0)
            colorState = 0; // Start over
        break;
    }

    // Small delay for visible color transitions
    delay(5);
}