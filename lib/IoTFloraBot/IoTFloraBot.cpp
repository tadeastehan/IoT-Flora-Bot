#include "IoTFloraBot.h"
#include "iot_configs.h"
#include <HttpLibrary.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SensorLibrary.h>

// ...existing code...

void setupSensors()
{
    setupMoistureSensor();
    setupADC();
    setupBME();
    setupRGB();
}

void establishConnection()
{
    // Connect to WiFi
    connectToWiFi(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);

    // Initialize NTP client
    initializeNTP();
}

String generateTelemetryPayload()
{
    String telemetry_payload = "";

    int percentage = getAveragePercentageWithPWM();
    int USBVoltage = ReadUSBVoltage();
    int batteryVoltage = ReadBatteryVoltage();
    int lightIntensity = calculateLightIntensity();
    float temperature = getTemperature();
    float humidity = getHumidity();
    float pressure = getPressure();
    float altitude = getAltitude();

    telemetry_payload = "\"{" +
                        String("\\\"Moisture\\\": ") + String(percentage) + ", " +
                        String("\\\"USBVoltage\\\": ") + String(USBVoltage) + ", " +
                        String("\\\"BatteryVoltage\\\": ") + String(batteryVoltage) + ", " +
                        String("\\\"LightIntensity\\\": ") + String(lightIntensity) + ", " +
                        String("\\\"Temperature\\\": ") + String(temperature) + ", " +
                        String("\\\"Humidity\\\": ") + String(humidity) + ", " +
                        String("\\\"Pressure\\\": ") + String(pressure) + ", " +
                        String("\\\"Altitude\\\": ") + String(altitude) +
                        "}\"";

    unsigned long epochTime = getEpochTime();
    String nanoTimestampStr = String(epochTime * 1000000000ULL);

    // Prepare the JSON payload
    String payload = R"(
      {
          "streams": [
          {
              "stream": {"source": ")";
    payload += IOT_CONFIG_DEVICE_ID; // Add device ID
    payload += R"("},
              "values": [
              [
                  ")";
    payload += nanoTimestampStr; // Add nanosecond timestamp
    payload += R"(", )";
    payload += telemetry_payload;
    payload += R"(
              ]
              ]
          }
          ]
      }
      )";

    return payload;
}

void sendTelemetry()
{
    String payload = generateTelemetryPayload();
    sendDataToGrafanaLoki(IOT_CONFIG_IOTHUB_FQDN, IOT_CONFIG_USERNAME, IOT_CONFIG_DEVICE_KEY, payload);
}
