#include <Arduino.h>
#include <driver/adc.h>

float read_voltage(adc1_channel_t channel, float R1, float R2)
{
    long sum = 0;        // Sum of samples taken
    float voltage = 0.0; // Calculated voltage

    for (int i = 0; i < 500; i++)
    {
        sum += adc1_get_raw(channel);
        delayMicroseconds(1000);
    }
    // Calculate the voltage
    voltage = sum / (float)500;
    Serial.print("Raw value: ");
    Serial.println(voltage);
    voltage = (voltage * 0.886) / 3917; // For internal 1.1v reference
    // Use it with divider circuit
    voltage = voltage / (R2 / (R1 + R2));
    // Round value by two DP
    voltage = roundf(voltage * 100) / 100;

    return voltage;
}

void setup()
{
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_0db); // Configure pin 1
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_0db); // Configure pin 2
    Serial.begin(115200);

    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);
}

void loop()
{

    float R1_channel_0 = 47000.0;
    float R2_channel_0 = 10000.0;

    float voltage1 = read_voltage(ADC1_CHANNEL_0, R1_channel_0, R2_channel_0); // Read from pin 1

    Serial.print("Voltage 1 (pin 1): ");
    Serial.println(voltage1, 2);

    float R1_channel_1 = 100000.0;
    float R2_channel_1 = 10000.0;

    float voltage2 = read_voltage(ADC1_CHANNEL_1, R1_channel_1, R2_channel_1); // Read from pin 2

    Serial.print("Voltage 2 (pin 2): ");
    Serial.println(voltage2, 2);
}
