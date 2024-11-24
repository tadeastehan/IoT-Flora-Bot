#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SDA 13 // SDA (Data) pin connected to pin 13
#define BME_SCL 14 // SCL (Clock) pin connected to pin 14
#define BME_EN 21  // Enable pin connected to pin 21 (to power the sensor)

#define BME_ADDR 0x76 // I2C address of BME280 sensor

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C communication

unsigned long delayTime;

void setup()
{
    // Start serial communication
    Serial.begin(115200);
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

    Serial.println("-- Default Test --");
    delayTime = 1000; // Delay between measurements
    Serial.println();
}

void loop()
{
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();

    delay(delayTime);
}
