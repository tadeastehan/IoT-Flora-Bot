#include <Arduino.h>
#include <IoTFloraBot.h>
#include "iot_configs.h"

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

void setup()
{
  Serial.begin(115200);
  delay(1000); // Take some time to open up the Serial Monitor

  esp_sleep_enable_timer_wakeup(TELEMETRY_FREQUENCY_SECS * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TELEMETRY_FREQUENCY_SECS) + " Seconds");

  // Establish connection
  setupSensors();

  // Send telemetry data if connection is established
  if (establishConnection())
  {
    sendTelemetry();
  }

  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop()
{
}
