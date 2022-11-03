#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

#define I2C_Freq 100000
#define OLED_SDA 32
#define OLED_SCL 33
TwoWire I2C_0 = TwoWire(0);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_0, OLED_RESET);
void setup()
{
    I2C_0.begin(OLED_SDA, OLED_SCL, I2C_Freq);

    Serial.begin(115200);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.drawPixel(32, 16, SSD1306_WHITE);
    display.drawPixel(32, 63, SSD1306_WHITE);
    display.drawPixel(95, 16, SSD1306_WHITE);
    display.drawPixel(95, 63, SSD1306_WHITE);
    display.display();

    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(32, 16);           // Start at top-left corner
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font

    pinMode(GPIO_NUM_21, INPUT_PULLUP);
}

void loop()
{
    if (!digitalRead((GPIO_NUM_21)))
    {
        display.write('a');
        display.display();
    }
}
