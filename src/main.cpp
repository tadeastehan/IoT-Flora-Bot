#include <Arduino.h>
#include <IoTFloraBot.h>
#include "iot_configs.h"

unsigned long previousMillis = 0; // To keep track of the last update time

void setup()
{
  Serial.begin(115200);

  // Establish connection
  setupSensors();
  establishConnection();
}

void loop()
{
  unsigned long currentMillis = millis();

  // Send data every 10 seconds
  if (currentMillis - previousMillis >= TELEMETRY_FREQUENCY_MILLISECS)
  {
    previousMillis = currentMillis;

    // Send telemetry data
    sendTelemetry();
  }
}
