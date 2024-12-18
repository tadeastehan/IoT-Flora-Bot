#ifndef SENSORLIBRARY_H
#define SENSORLIBRARY_H

#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <FastLED.h>

// Function declarations
void initialize_adc();
float ReadVoltage(adc1_channel_t channel, float R1, float R2);
float ReadUSBVoltage();
float ReadBatteryVoltage();
float ReadPhotodiodeVoltage();
int calculateLightIntensity();
int mapMoistureToPercentage(int moistureValue);
int getSmoothedReading(int newReading);
void updateMoistureValue();
int getAveragePercentage();
float getHumidity();
float getTemperature();
float getPressure();
float getAltitude();
void setupMoistureSensor();
void setupADC();
void setupBME();
void setBrightness(int brightness);
void setupRGB();
void setLEDColor(int r, int g, int b);
void turnOffLED();
void startPWM(int pin, int frequency, int dutyCycle);
void stopPWM(int pin);
int getAveragePercentageWithPWM();

#endif // SENSORLIBRARY_H
