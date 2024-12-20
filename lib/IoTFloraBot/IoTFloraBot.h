#ifndef IOTFLORABOT_H
#define IOTFLORABOT_H

#include <Arduino.h>

void setupSensors();
void establishConnection();
String generateTelemetryPayload();
void sendTelemetry();

#endif // IOTFLORABOT_H
