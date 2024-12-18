#include <Arduino.h>
#include <WiFi.h>
#include <IoTFloraBot.h>
#include <iot_configs.h>

static unsigned long next_telemetry_send_time_ms = 0;

void setup()
{
  establishConnection();

  Serial.begin(115200); // init serial monitor
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToWiFi();
  }

  else if (millis() > next_telemetry_send_time_ms)
  {
    sendTelemetry();
    next_telemetry_send_time_ms = millis() + TELEMETRY_FREQUENCY_MILLISECS;
  }
}