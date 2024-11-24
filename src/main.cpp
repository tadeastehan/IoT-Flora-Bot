#include <Arduino.h>

void setup() {
    // Initialize USB CDC
    Serial.begin(115200); // This initializes USB CDC as the primary serial interface
    delay(1000);
    Serial.println("USB CDC Enabled!");
}

void loop() {
    // Print a message every second
    Serial.println("Hello World!");
    delay(1000);
}