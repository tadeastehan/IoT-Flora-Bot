#ifndef HTTPLIBRARY_H
#define HTTPLIBRARY_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Function declarations
bool connectToWiFi(const char *ssid, const char *password);
void initializeNTP();
unsigned long getEpochTime();
void sendDataToGrafanaLoki(const char *url, const char *user, const char *apiKey, const String &payload);

#endif // HTTPLIBRARY_H
