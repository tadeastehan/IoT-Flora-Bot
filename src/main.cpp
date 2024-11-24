#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Default Vref in mV (set to a typical value for ESP32)
#define DEFAULT_VREF 1100

// ADC characteristics structure
esp_adc_cal_characteristics_t *adc_chars;

#define BATT_EN_PIN 3

void initialize_adc()
{
  // Allocate memory for ADC characteristics
  adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

  // Check eFuse calibration availability
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
  {
    Serial.println("eFuse Two Point calibration data available.");
  }
  else if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
  {
    Serial.println("eFuse Vref calibration data available.");
  }
  else
  {
    Serial.println("Default Vref will be used.");
  }

  // Characterize ADC
  esp_adc_cal_value_t cal_type = esp_adc_cal_characterize(
      ADC_UNIT_1,       // ADC unit
      ADC_ATTEN_DB_0,   // Attenuation
      ADC_WIDTH_BIT_12, // Bit width
      DEFAULT_VREF,     // Default Vref in mV
      adc_chars);       // ADC characteristics structure

  // Print the type of calibration used
  if (cal_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    Serial.println("Characterized using Two Point values stored in eFuse.");
  }
  else if (cal_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    Serial.println("Characterized using reference voltage stored in eFuse.");
  }
  else
  {
    Serial.println("Characterized using default reference voltage.");
  }
}

void ReadVoltage(adc1_channel_t channel, float R1, float R2)
{
  uint32_t adc_reading = 0;
  uint32_t voltage = 0;

  // Take multiple samples for better accuracy
  for (int i = 0; i < 100; i++)
  {
    adc_reading += adc1_get_raw(channel);
  }
  adc_reading /= 100;

  // Convert raw ADC reading to voltage in mV
  voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

  // Calculate the real voltage using the voltage divider
  float real_voltage = (float(voltage) * (R1 + R2)) / R2;

  // Print results
  Serial.print("Channel ");
  Serial.print(channel);
  Serial.print(" - ADC Reading: ");
  Serial.println(adc_reading);
  Serial.print("Calibrated Voltage (mV): ");
  Serial.println(voltage);
  Serial.print("Real Voltage (V): ");
  Serial.println(real_voltage, 2);
}

void setup()
{
  // Initialize GPIO
  pinMode(BATT_EN_PIN, OUTPUT);

  Serial.begin(115200);

  // Initialize ADC
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_0);

  // Initialize calibration
  initialize_adc();
}

void loop()
{
  // Read USB Voltage (e.g., with divider R1=47K, R2=10K)
  ReadVoltage(ADC1_CHANNEL_0, 47000.0, 10000.0);

  delay(1000);
  // Read Battery Voltage (e.g., with divider R1=100K, R2=10K)
  digitalWrite(BATT_EN_PIN, HIGH);
  ReadVoltage(ADC1_CHANNEL_1, 100000.0, 10000.0);
  digitalWrite(BATT_EN_PIN, LOW);

  delay(1000);
}
