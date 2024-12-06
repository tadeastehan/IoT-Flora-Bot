#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Default Vref in mV (adjust based on your ESP32 calibration data)
#define DEFAULT_VREF 1100

// Pins
#define MOISTURE_IN_PIN 5                   // GPIO5: Generate the PWM signal
#define MOISTURE_OUT_CHANNEL ADC1_CHANNEL_3 // GPIO4: Read the sensor output via ADC1_CHANNEL_0 (mapped to GPIO36)

// Soil moisture calibration values
#define AIR_VALUE 3798   // ADC value for dry soil (calibrate this)
#define WATER_VALUE 2347 // ADC value for wet soil (calibrate this)

// ADC characteristics
esp_adc_cal_characteristics_t *adc_chars;

// Initialize the ADC for the ESP32
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
  esp_adc_cal_characteristics_t adc_characteristics;
  esp_adc_cal_characterize(
      ADC_UNIT_1,       // ADC unit
      ADC_ATTEN_DB_11,  // Attenuation (up to 3.3V input range)
      ADC_WIDTH_BIT_12, // 12-bit width
      DEFAULT_VREF,     // Default Vref in mV
      &adc_characteristics);
}

void setup()
{
  Serial.begin(115200);

  // Initialize moisture input pin
  pinMode(MOISTURE_IN_PIN, OUTPUT);

  // Initialize ADC
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(MOISTURE_OUT_CHANNEL, ADC_ATTEN_DB_12); // Attenuation for higher range (up to 3.3V)
  initialize_adc();
}

void loop()
{
  // Generate PWM signal on MOISTURE_IN
  analogWrite(MOISTURE_IN_PIN, 128); // Set a duty cycle of 50% (analogWrite uses software PWM on GPIO5)

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

  // Convert raw ADC reading to voltage in mV
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

  // Map ADC value to soil moisture percentage
  int soil_moisture = map(adc_reading, AIR_VALUE, WATER_VALUE, 0, 100);
  soil_moisture = constrain(soil_moisture, 0, 100); // Ensure values stay within range

  // Print results
  Serial.print("ADC Reading: ");
  Serial.println(adc_reading);
  Serial.print("Voltage (mV): ");
  Serial.println(voltage);
  Serial.print("Soil Moisture (%): ");
  Serial.println(soil_moisture);

  // Wait before next measurement
  delay(10000); // 10 seconds between measurements
}
