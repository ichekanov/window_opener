// #include "../lib/test/button_test.cpp"
// #include "../lib/test/driver_connection_test.cpp"
// #include "../lib/test/i2c_scan.cpp"
// #include "../lib/test/interrupt_test.cpp"
// #include "../lib/test/motor_manual_operation_test.cpp"
// #include "../lib/test/oled_test.cpp"
// #include "../lib/test/wifi_test.cpp"
// #include "../lib/test/http_get_test.cpp"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <Secrets.h>
#include <WiFi.h>
#include <Window.h>
#include <Wire.h>

int i;

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

#define FLOAT_TO_STRING(f) String(f).substring(0, String(f).indexOf(".") + 2).c_str()

#define TMC2209_RXD 16
#define TMC2209_TXD 17
#define TMC2209_STEP_PIN 19
Window window;

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

float getTemperature();
bool connectWiFi(const char *, const char *);

void setup()
{
    pinMode(BUTTON, INPUT_PULLUP);

    I2C_0.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);
    OLED_CLEAR();

    delay(5000);

    OLED_PRINT_LINE("Starting...", 0);
    delay(250);

    OLED_PRINT_LINE("Driver", 1);
    if (!window.begin(Serial2, TMC2209_STEP_PIN))
    {
        OLED_PRINT_LINE("       bad", 1);
        // OLED_PRINT_LINE("Cant reach", 2);
        // OLED_PRINT_LINE("Reboot...", 5);
        // delay(2000);
        // exit(1);
    }
    OLED_PRINT_LINE("        OK", 1);
    window.disable();
    delay(250);

    OLED_PRINT_LINE("WiFi", 2);
    if (!connectWiFi(ssid, password))
    {
        OLED_PRINT_LINE("Cant reach", 3);
        OLED_PRINT_LINE("Reboot...", 5);
        delay(2000);
        exit(1);
    }
    OLED_PRINT_LINE("        OK", 2);
    window.enable();
    delay(250);

    OLED_PRINT_LINE("t", 3);
    display.write(248);
    display.display();
    auto temperature = getTemperature();
    if (temperature == 404.)
    {
        OLED_PRINT_LINE("Cant reach", 4);
        OLED_PRINT_LINE("Reboot...", 5);
        delay(2000);
        exit(1);
    }
    OLED_PRINT_LINE((String("      ") + FLOAT_TO_STRING(temperature)).c_str(), 3);
    delay(250);

    OLED_PRINT_LINE("Ready!", 5);
    delay(2000);
}

void loop()
{
    // //* testing stallguard
    // OLED_CLEAR_PRINT("Closing...");
    // auto m = window.close();
    // OLED_CLEAR();
    // OLED_ADD_LINE("Closed!", 0);
    // OLED_ADD_LINE("STALLGUARD", 4);
    // OLED_PRINT_LINE(String(m).c_str(), 5);
    // window.moveManually(110);

    // //* testing temperature getter
    auto r = getTemperature();
    OLED_CLEAR_PRINT(FLOAT_TO_STRING(r));
    delay(1000);
}

bool connectWiFi(const char *ssid, const char *password)
{
    delay(1000);
    WiFi.begin(ssid, password);
    for (int j = 0; j < 30; ++j)
    {
        delay(1000);
        if (WiFi.status() == WL_CONNECTED)
            return 1;
    }
    return 0;
}

float getTemperature()
{
    float result;
    if ((WiFi.status() == WL_CONNECTED))
    {
        HTTPClient http;
        http.begin(server);
        int httpCode = http.GET();
        if (httpCode > 0)
            result = http.getString().toFloat(); // actual temperature
        else
            result = 404.; // server is not responding.
        http.end();
    }
    else
        result = 408.; // no WiFi connection
    return result;
}
