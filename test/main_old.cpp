#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <FastLED.h>

// RGB LED setup
#define NUM_LEDS 1
#define LED_PIN 47
#define BRIGHTNESS 10
CRGB leds[NUM_LEDS];

// Moisture setup
#define MOISTURE_IN_PIN 5
#define MOISTURE_OUT_PIN GPIO_NUM_4
#define WINDOW_SIZE 10 // Number of readings to average
int readings[WINDOW_SIZE];
int readIndex = 0;
int totalReadings = 0;
bool bufferFilled = false;

// Voltage reading setup
#define BATT_EN_PIN 3
#define PHOTODIODE_EN_PIN 9

#define USB_VOLTAGE_CHANNEL ADC1_CHANNEL_0
#define BATT_VOLTAGE_CHANNEL ADC1_CHANNEL_1
#define PHOTODIODE_VOLTAGE_CHANNEL ADC1_CHANNEL_9

#define USB_VOLTAGE_R1 47000.0
#define USB_VOLTAGE_R2 10000.0

#define BATT_VOLTAGE_R1 100000.0
#define BATT_VOLTAGE_R2 10000.0

#define PHOTODIODE_VOLTAGE_R1 0
#define PHOTODIODE_VOLTAGE_R2 10000.0

#define ATTENTUATION ADC_ATTEN_DB_0
#define WIDTH ADC_WIDTH_BIT_12
#define DEFAULT_VREF 1100
esp_adc_cal_characteristics_t *adc_chars;

// BME280 setup
#define BME_SDA 13    // SDA (Data) pin connected to pin 13
#define BME_SCL 14    // SCL (Clock) pin connected to pin 14
#define BME_EN 21     // Enable pin connected to pin 21 (to power the sensor)
#define BME_ADDR 0x76 // I2C address of BME280 sensor
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C communication

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
        ADC_UNIT_1,   // ADC unit
        ATTENTUATION, // Attenuation
        WIDTH,        // Bit width
        DEFAULT_VREF, // Default Vref in mV
        adc_chars);   // ADC characteristics structure

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

float ReadUSBVoltage()
{
    return ReadVoltage(USB_VOLTAGE_CHANNEL, USB_VOLTAGE_R1, USB_VOLTAGE_R2);
}

float ReadBatteryVoltage()
{
    digitalWrite(BATT_EN_PIN, HIGH);
    return ReadVoltage(BATT_VOLTAGE_CHANNEL, BATT_VOLTAGE_R1, BATT_VOLTAGE_R2);
    digitalWrite(BATT_EN_PIN, LOW);
}

float ReadPhotodiodeVoltage()
{
    digitalWrite(PHOTODIODE_EN_PIN, HIGH);
    return ReadVoltage(PHOTODIODE_VOLTAGE_CHANNEL, PHOTODIODE_VOLTAGE_R1, PHOTODIODE_VOLTAGE_R2);
    digitalWrite(PHOTODIODE_EN_PIN, LOW);
}

int calculateLightIntensity()
{
    // Constants from the datasheet
    const float referenceCurrent = 11e-6; // 11 µA at Ee = 1 mW/cm²
    const float referenceIntensity = 1.0; // 1 mW/cm² at the reference current

    // Calculate the photocurrent (IL)
    float voltage = ReadPhotodiodeVoltage();

    float photocurrent = voltage / 10000.0; // 10kΩ resistor

    // Calculate light intensity (Ee)
    float lightIntensity = (photocurrent / referenceCurrent) * referenceIntensity;
    return lightIntensity;
}

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

float getHumidity()
{
    return bme.readHumidity();
}

float getTemperature()
{
    return bme.readTemperature();
}

float getPressure()
{
    return bme.readPressure();
}

float getAltitude()
{
    return bme.readAltitude(SEALEVELPRESSURE_HPA);
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

void setupADC()
{
    pinMode(BATT_EN_PIN, OUTPUT);
    pinMode(PHOTODIODE_EN_PIN, OUTPUT);

    adc1_config_width(WIDTH);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ATTENTUATION);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ATTENTUATION);
    adc1_config_channel_atten(ADC1_CHANNEL_9, ATTENTUATION);

    initialize_adc();
}

void setupBME()
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
        delay(1000);
    }
}

void setBrightness(int brightness)
{
    FastLED.setBrightness(brightness);
}

void setupRGB()
{
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    setBrightness(BRIGHTNESS);
}

void setLEDColor(int r, int g, int b)
{
    leds[0] = CRGB(r, g, b);
    FastLED.show();
}

void turnOffLED()
{
    leds[0] = CRGB(0, 0, 0);
    FastLED.show();
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

void setup()
{
    Serial.begin(115200);

    setupMoistureSensor();
    setupADC();
    setupBME();
    setupRGB();
}

void loop()
{
    int percentage = getAveragePercentageWithPWM();
    int USBVoltage = ReadUSBVoltage();
    int batteryVoltage = ReadBatteryVoltage();
    int lightIntensity = calculateLightIntensity();
    float temperature = getTemperature();
    float humidity = getHumidity();
    float pressure = getPressure();
    float altitude = getAltitude();

    Serial.print(">");
    Serial.print("Moisture:");
    Serial.print(percentage);
    Serial.print(",");
    Serial.print("USBVoltage:");
    Serial.print(USBVoltage);
    Serial.print(",");
    Serial.print("BatteryVoltage:");
    Serial.print(batteryVoltage);
    Serial.print(",");
    Serial.print("LightIntensity:");
    Serial.println(lightIntensity);

    Serial.print(">");
    Serial.print("Temperature:");
    Serial.print(temperature);
    Serial.print(",");
    Serial.print("Humidity:");
    Serial.print(humidity);
    Serial.print(",");
    Serial.print("Pressure:");
    Serial.print(pressure);
    Serial.print(",");
    Serial.print("Altitude:");
    Serial.print(altitude);
    Serial.println();

    setLEDColor(255, 0, 0);
    delay(1000);
    setLEDColor(0, 255, 0);
    delay(1000);
    setLEDColor(0, 0, 255);
    delay(1000);
    turnOffLED();

    // Wait before next measurement
    delay(1000); // 10 seconds between measurements
}
