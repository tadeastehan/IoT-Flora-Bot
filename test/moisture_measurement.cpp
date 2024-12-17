#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Pins
#define MOISTURE_IN_PIN 5
#define MOISTURE_OUT_PIN GPIO_NUM_4

#define WINDOW_SIZE 10 // Number of readings to average
int readings[WINDOW_SIZE];
int readIndex = 0;
int totalReadings = 0;
bool bufferFilled = false;

int mapMoistureToPercentage(int moistureValue)
{
  if (moistureValue >= 3795)
    return 0;
  if (moistureValue <= 3758)
    return 100;

  if (moistureValue >= 3787)
    return map(moistureValue, 3787, 3795, 25, 0);
  if (moistureValue >= 3785)
    return map(moistureValue, 3785, 3787, 50, 25);
  if (moistureValue >= 3769)
    return map(moistureValue, 3769, 3785, 75, 50);
  return map(moistureValue, 3758, 3769, 100, 75);
}

int getSmoothedReading(int newReading)
{
  totalReadings = totalReadings - readings[readIndex];
  readings[readIndex] = newReading;
  totalReadings = totalReadings + readings[readIndex];
  readIndex = (readIndex + 1) % WINDOW_SIZE;

  if (readIndex == 0)
  {
    bufferFilled = true;
  }

  if (!bufferFilled)
  {
    return newReading;
  }

  return totalReadings / WINDOW_SIZE;
}

void updateMoistureValue()
{
  int rawMoistureValue = analogRead(MOISTURE_OUT_PIN);
  int smoothedValue = getSmoothedReading(rawMoistureValue);

  Serial.print(">");
  Serial.print("moisture:");
  Serial.print(smoothedValue);
  Serial.print(",");
  Serial.print("percentage:");
  Serial.print(mapMoistureToPercentage(smoothedValue));
  Serial.println();
}

int getAveragePercentage()
{
  int total = 0;
  for (int i = 0; i < 10; i++)
  {
    int rawMoistureValue = analogRead(MOISTURE_OUT_PIN);
    total += rawMoistureValue;
    delay(10); // Small delay between readings
  }
  int averageMoistureValue = total / 10;
  return mapMoistureToPercentage(averageMoistureValue);
}

void setupMoistureSensor()
{
  // Initialize moisture input pin
  pinMode(MOISTURE_IN_PIN, OUTPUT);
  pinMode(MOISTURE_OUT_PIN, INPUT);

  for (int i = 0; i < WINDOW_SIZE; i++)
  {
    readings[i] = 0;
  }
}

void setup()
{
  Serial.begin(115200);

  setupMoistureSensor();
}

void startPWM(int pin, int frequency, int dutyCycle)
{
  ledcSetup(0, frequency, 3); // Channel 0, frequency, 8-bit resolution
  ledcAttachPin(pin, 0);      // Attach channel 0 to the specified pin
  ledcWrite(0, dutyCycle);    // Set duty cycle (0-255)
}

void stopPWM(int pin)
{
  ledcDetachPin(pin);
}

int getAveragePercentageWithPWM()
{
  startPWM(MOISTURE_IN_PIN, 1500000, 86); // Generate PWM signal on MOISTURE_IN
  delay(100);

  int total = 0;
  for (int i = 0; i < 10; i++)
  {
    int rawMoistureValue = analogRead(MOISTURE_OUT_PIN);
    total += rawMoistureValue;
    delay(10); // Small delay between readings
  }
  stopPWM(MOISTURE_IN_PIN);

  int averageMoistureValue = total / 10;
  return mapMoistureToPercentage(averageMoistureValue);
}

void loop()
{
  int percentage = getAveragePercentageWithPWM();

  Serial.print(">");
  Serial.print("percentage:");
  Serial.print(percentage);
  Serial.println();

  // Wait before next measurement
  delay(100); // 10 seconds between measurements
}
