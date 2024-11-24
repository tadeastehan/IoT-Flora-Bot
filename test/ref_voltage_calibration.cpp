#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Default Vref in mV, used if no eFuse values are available
#define DEFAULT_VREF 1100

// ADC characteristics structure
esp_adc_cal_characteristics_t *adc_chars;

void setup()
{
    Serial.begin(115200);

    // Allocate memory for ADC characteristics
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

    // Check if eFuse Vref or Two Point values are burned into eFuse
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

    // Configure ADC1 Channel 0 (GPIO36) with attenuation of 0dB
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);

    // Characterize ADC
    esp_adc_cal_value_t cal_type = esp_adc_cal_characterize(
        ADC_UNIT_1,       // ADC unit
        ADC_ATTEN_DB_0,   // Attenuation
        ADC_WIDTH_BIT_12, // Bit width
        DEFAULT_VREF,     // Default Vref in mV
        adc_chars);       // ADC characteristics structure

    // Print type of calibration used
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

void loop()
{
    uint32_t adc_reading = 0;
    uint32_t voltage = 0;

    // Take multiple samples for better accuracy
    for (int i = 0; i < 100; i++)
    {
        adc_reading += adc1_get_raw(ADC1_CHANNEL_0);
    }
    adc_reading /= 100;

    // Convert raw ADC reading to voltage in mV
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    // Print results
    Serial.print("ADC Reading: ");
    Serial.println(adc_reading);
    Serial.print("Voltage (mV): ");
    Serial.println(voltage);

    delay(1000); // Delay for 1 second
}
