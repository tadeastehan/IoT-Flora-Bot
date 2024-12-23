#ifndef IOTFLORABOT_H
#define IOTFLORABOT_H

#include <Arduino.h>

void setupSensors();
bool establishConnection();
String generateTelemetryPayload();
void sendTelemetry();

#endif // IOTFLORABOT_H
