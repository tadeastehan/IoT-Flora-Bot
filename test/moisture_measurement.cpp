#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Pins
#define MOISTURE_IN_PIN 5
#define MOISTURE_OUT_CHANNEL ADC1_CHANNEL_3
#define MOISTURE_ATTENUATION ADC_ATTEN_DB_12

// Soil moisture calibration values
#define AIR_VALUE 3798   // ADC value for dry soil (calibrate this)
#define WATER_VALUE 2347 // ADC value for wet soil (calibrate this)

void setup()
{
  Serial.begin(115200);

  // Initialize moisture input pin
  pinMode(MOISTURE_IN_PIN, OUTPUT);

  // Initialize ADC
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(MOISTURE_OUT_CHANNEL, MOISTURE_ATTENUATION); // Attenuation for higher range (up to 3.3V)
}

void setupPWM(int pin, int frequency, int dutyCycle)
{
  ledcSetup(0, frequency, 8); // Channel 0, frequency, 8-bit resolution
  ledcAttachPin(pin, 0);      // Attach channel 0 to the specified pin
  ledcWrite(0, dutyCycle);    // Set duty cycle (0-255)
}

void loop()
{
  setupPWM(MOISTURE_IN_PIN, 1000, 128); // Generate PWM signal on MOISTURE_IN
  // Allow the circuit to stabilize
  delay(200);

  // Read the voltage from MOISTURE_OUT
  uint32_t adc_reading = 0;
  for (int i = 0; i < 5; i++)
  { // Average over 5 samples for accuracy
    adc_reading += adc1_get_raw(MOISTURE_OUT_CHANNEL);
    delay(1000); // 1 second between readings
  }
  adc_reading /= 5;

  // Map ADC value to soil moisture percentage
  int soil_moisture = map(adc_reading, AIR_VALUE, WATER_VALUE, 0, 100);
  soil_moisture = constrain(soil_moisture, 0, 100); // Ensure values stay within range

  // Print results
  Serial.print("ADC Reading: ");
  Serial.println(adc_reading);
  Serial.print("Soil Moisture (%): ");
  Serial.println(soil_moisture);

  setupPWM(MOISTURE_IN_PIN, 1000, 0);
  // Wait before next measurement
  delay(10000); // 10 seconds between measurements
}
