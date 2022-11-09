#include <Arduino.h>
#include <Wire.h>

#define I2C_Freq 100000

#define OLED_SDA 32
#define OLED_SCL 33

TwoWire I2C_0 = TwoWire(0);

void setup()
{
    I2C_0.begin(OLED_SDA, OLED_SCL, I2C_Freq);
    Serial.begin(115200);
    Serial.println("\nI2C Scanner");
}

void loop()
{
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        I2C_0.beginTransmission(address);
        error = I2C_0.endTransmission();
        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }
    delay(5000);
}
