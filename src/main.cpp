#include <Arduino.h>
#include <IoTFloraBot.h>
#include "iot_configs.h"

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

#define DEEP_SLEEP false // Set to true to enable deep sleep

void setup()
{
  Serial.begin(115200);
  delay(1000); // Take some time to open up the Serial Monitor

  if (DEEP_SLEEP)
  {
    Serial.println("Deep Sleep Enabled");
    esp_sleep_enable_timer_wakeup(TELEMETRY_FREQUENCY_SECS * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TELEMETRY_FREQUENCY_SECS) + " Seconds");
  }
  else
  {
    Serial.println("Deep Sleep Disabled");
  }

  // Establish connection
  setupSensors();

  // Send telemetry data if connection is established
  if (establishConnection())
  {
    sendTelemetry();
  }

  if (DEEP_SLEEP)
  {
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
  }
  else
  {
    Serial.println("Not going to sleep, waiting for next cycle...");
  }
}

void loop()
{
  // Send telemetry data if connection is established
  if (establishConnection())
  {
    sendTelemetry();
  }
  delay(TELEMETRY_FREQUENCY_SECS_INTERVAL * 1000); // Sleep for the specified time
}
