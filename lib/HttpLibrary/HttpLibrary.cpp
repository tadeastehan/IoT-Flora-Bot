#include "HttpLibrary.h"

// NTP Client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC timezone, update every 60 seconds

void connectToWiFi(const char *ssid, const char *password)
{
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

void initializeNTP()
{
    timeClient.begin();
}

unsigned long getEpochTime()
{
    if (!timeClient.update())
    {
        timeClient.forceUpdate();
    }
    return timeClient.getEpochTime();
}

void sendDataToGrafanaLoki(const char *url, const char *user, const char *apiKey, const String &payload)
{
    HTTPClient http;
    http.begin(url);
    http.setAuthorization(user, apiKey);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 204)
    {
        Serial.println("Success");
    }
    else
    {
        Serial.printf("Error: %d, Response: %s\n", httpResponseCode, http.getString().c_str());
    }

    http.end();
}
