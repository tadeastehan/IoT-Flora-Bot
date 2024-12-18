#include <Arduino.h>
#include <WiFi.h>
#include <IoTFloraBot.h>
#include <iot_configs.h>
#include <SensorLibrary.h>

static unsigned long next_telemetry_send_time_ms = 0;

void setup()
{
  setupMoistureSensor();
  setupADC();
  setupBME();
  setupRGB();

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

  // Wait before next measurement
  delay(1000); // 10 seconds between measurements
}