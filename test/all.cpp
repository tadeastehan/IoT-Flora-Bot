#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

#define BME_SDA 13 // SDA (Data) pin connected to pin 13
#define BME_SCL 14 // SCL (Clock) pin connected to pin 14
#define BME_EN 21  // Enable pin connected to pin 21 (to power the sensor)

#define BME_ADDR 0x76 // I2C address of BME280 sensor

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C communication

#define DEFAULT_VREF 1100
esp_adc_cal_characteristics_t *adc_chars;
esp_adc_cal_characteristics_t *adc_chars_moisture;

#define BATT_EN_PIN 3
#define PHOTODIODE_EN_PIN 9

#define MOISTURE_IN_PIN 5                   // GPIO5: Generate the PWM signal
#define MOISTURE_OUT_CHANNEL ADC1_CHANNEL_3 // GPIO4: Read the sensor output via ADC1_CHANNEL_0 (mapped to GPIO36)

// Soil moisture calibration values
#define AIR_VALUE 3798   // ADC value for dry soil (calibrate this)
#define WATER_VALUE 2347 // ADC value for wet soil (calibrate this)

int soilMoisturePercentage;

float usbVoltage;
float batteryVoltage;
float photodiodeVoltage;
float lightIntensity;

float temperature, humidity, pressure, altitude;

void initBME280()
{
    // Enable BME EN pin to power the sensor
    pinMode(BME_EN, OUTPUT);
    digitalWrite(BME_EN, HIGH); // Turn on the sensor

    // Initialize I2C with custom pins
    Wire.begin(BME_SDA, BME_SCL); // SDA on pin 13, SCL on pin 14

    unsigned status;

    // Initialize BME280 sensor (I2C mode)
    status = bme.begin(BME_ADDR, &Wire);
    if (!status)
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x");
        Serial.println(bme.sensorID(), 16);
        while (1)
            delay(10); // Halt if sensor is not found
    }
}

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

void initialize_adc_moisture()
{
    // Allocate memory for ADC characteristics
    adc_chars_moisture = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

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
        ADC_UNIT_1,          // ADC unit
        ADC_ATTEN_DB_11,     // Attenuation for higher range (up to 3.3V)
        ADC_WIDTH_BIT_12,    // Bit width
        DEFAULT_VREF,        // Default Vref in mV
        adc_chars_moisture); // ADC characteristics structure

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

float ReadVoltage(adc1_channel_t channel, float R1, float R2)
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

    return real_voltage;
}

void readUSBVoltage()
{
    usbVoltage = ReadVoltage(ADC1_CHANNEL_0, 47000.0, 10000.0);
}

void readBatteryVoltage()
{
    digitalWrite(BATT_EN_PIN, HIGH);
    batteryVoltage = ReadVoltage(ADC1_CHANNEL_1, 100000.0, 10000.0);
    digitalWrite(BATT_EN_PIN, LOW);
}

float readPhotodiodeVoltage()
{
    digitalWrite(PHOTODIODE_EN_PIN, HIGH);
    photodiodeVoltage = ReadVoltage(ADC1_CHANNEL_9, 0, 10000.0); // 0 - 925mV
    digitalWrite(PHOTODIODE_EN_PIN, LOW);

    return photodiodeVoltage;
}

void calculateLightIntensity()
{
    // Constants from the datasheet
    const float referenceCurrent = 11e-6; // 11 µA at Ee = 1 mW/cm²
    const float referenceIntensity = 1.0; // 1 mW/cm² at the reference current

    // Calculate the photocurrent (IL)
    float voltage = readPhotodiodeVoltage();

    float photocurrent = voltage / 10000.0; // 10kΩ resistor

    // Calculate light intensity (Ee)
    lightIntensity = (photocurrent / referenceCurrent) * referenceIntensity;
}

void readSoilMoisture()
{
    // Generate PWM signal on MOISTURE_IN
    analogWrite(MOISTURE_IN_PIN, 128); // Set a duty cycle of 50% (analogWrite uses software PWM on GPIO5)
    Serial.println("PWM signal generated on MOISTURE_IN_PIN");

    // Allow the circuit to stabilize
    delay(1000);

    // Read the voltage from MOISTURE_OUT
    uint32_t adc_reading = 0;
    for (int i = 0; i < 5; i++)
    { // Average over 5 samples for accuracy
        adc_reading += adc1_get_raw(MOISTURE_OUT_CHANNEL);
        delay(1000); // 1 second between readings
    }
    adc_reading /= 5;

    // Convert raw ADC reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars_moisture);

    // Map ADC value to soil moisture percentage
    soilMoisturePercentage = map(adc_reading, AIR_VALUE, WATER_VALUE, 0, 100);
    soilMoisturePercentage = constrain(soilMoisturePercentage, 0, 100); // Ensure values stay within range

    // Print results
    Serial.print("ADC Reading: ");
    Serial.println(adc_reading);
    Serial.print("Voltage (mV): ");
    Serial.println(voltage);
    Serial.print("Soil Moisture (%): ");
    Serial.println(soilMoisturePercentage);

    analogWrite(MOISTURE_IN_PIN, 0); // Turn off the PWM signal
}

void readBME280()
{
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void setup()
{
    // Start serial communication
    Serial.begin(115200);
    initBME280();
    Serial.println("-- Default Test --");
    Serial.println();

    // Initialize GPIO
    pinMode(BATT_EN_PIN, OUTPUT);
    pinMode(PHOTODIODE_EN_PIN, OUTPUT);
    pinMode(MOISTURE_IN_PIN, OUTPUT);

    // Initialize ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_0);
    adc1_config_channel_atten(ADC1_CHANNEL_9, ADC_ATTEN_DB_0);

    // Initialize ADC calibration
    initialize_adc();

    // Initialize ADC for moisture sensor with different attenuation
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11); // Attenuation for higher range (up to 3.3V)
    initialize_adc_moisture();
}

void loop()
{
    Serial.println("Reading BME280 sensor...");
    readBME280();
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println(" m");

    delay(1000);

    Serial.println("Reading USB Voltage...");
    readUSBVoltage();
    Serial.print("USB Voltage: ");
    Serial.println(usbVoltage, 2);

    delay(1000);

    Serial.println("Reading Battery Voltage...");
    readBatteryVoltage();
    Serial.print("Battery Voltage: ");
    Serial.println(batteryVoltage, 2);

    delay(1000);

    Serial.println("Calculating Light Intensity...");
    calculateLightIntensity();
    Serial.print("Light Intensity: ");
    Serial.print(lightIntensity, 2);
    Serial.println(" mW/cm²");

    delay(1000);

    Serial.println("Reading Soil Moisture...");
    readSoilMoisture();

    delay(10000);
}
