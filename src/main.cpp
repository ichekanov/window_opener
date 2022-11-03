// #include "../lib/test/button_test.cpp"
// #include "../lib/test/driver_connection_test.cpp"
// #include "../lib/test/i2c_scan.cpp"
// #include "../lib/test/interrupt_test.cpp"
// #include "../lib/test/motor_manual_operation_test.cpp"
// #include "../lib/test/oled_test.cpp"
// #include "../lib/test/wifi_test.cpp"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Window.h>
#include <Wire.h>

int i, n;
int8_t driverMode = 0;
unsigned long prevBtnClick = 0;

#define OLED_ADD_LINE(s, line)                                                                                         \
    display.setCursor(32, 16 + 8 * line);                                                                              \
    for (i = 0; i < strlen(s); i++)                                                                                    \
        display.write(s[i]);

#define OLED_SHOW() display.display();

#define OLED_PRINT_LINE(s, line)                                                                                       \
    display.setCursor(32, 16 + 8 * line);                                                                              \
    for (i = 0; i < strlen(s); i++)                                                                                    \
        display.write(s[i]);                                                                                           \
    display.display();

#define OLED_CLEAR()                                                                                                   \
    display.clearDisplay();                                                                                            \
    display.display();

#define OLED_CLEAR_PRINT(s)                                                                                            \
    display.clearDisplay();                                                                                            \
    display.setCursor(32, 16);                                                                                         \
    for (i = 0; i < strlen(s); i++)                                                                                    \
        display.write(s[i]);                                                                                           \
    display.display();

#define TMC2209_RXD 16
#define TMC2209_TXD 17
#define TMC2209_STEP_PIN 19
Window window(Serial2, TMC2209_STEP_PIN);

#define BUTTON 21
#define DEBOUNCE_DELAY 250

#define OLED_SDA 32
#define OLED_SCL 33
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_FREQ 100000
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
TwoWire I2C_0 = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_0, OLED_RESET);

void setup()
{
    pinMode(BUTTON, INPUT_PULLUP);

    I2C_0.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);

    display.clearDisplay();
    display.drawPixel(32, 16, SSD1306_WHITE);
    display.drawPixel(32, 63, SSD1306_WHITE);
    display.drawPixel(95, 16, SSD1306_WHITE);
    display.drawPixel(95, 63, SSD1306_WHITE);
    display.display();

    OLED_PRINT_LINE("Checkup...", 0);
    OLED_PRINT_LINE("Driver OK", 1);
    OLED_PRINT_LINE((String("M/steps ") + String(window.getMicrostepsPerStep())).c_str(), 2);
    OLED_PRINT_LINE("Ready!", 5);
    delay(500);
}

void loop()
{
    OLED_CLEAR_PRINT("Closing...");
    auto m = window.close();
    OLED_CLEAR();
    OLED_ADD_LINE("Closed!", 0);
    OLED_ADD_LINE("STALLGUARD", 4);
    OLED_PRINT_LINE(String(m).c_str(), 5);
    window.moveManually(110);
}
