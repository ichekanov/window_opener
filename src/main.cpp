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

#define L(line) 16 + 8 * line
#define C(col) 32 + 6 * col

#define MIN_TEMPERATURE 19.5
#define MAX_TEMPERATURE 25.
#define MOVE_THRESHOLD 5

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
int16_t calculatePosition(float);

void setup()
{
    pinMode(BUTTON, INPUT_PULLUP);

    I2C_0.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);
    display.clearDisplay();
    display.display();

    display.setCursor(C(0), L(0));
    display.print("Driver");
    if (!window.begin(Serial2, TMC2209_STEP_PIN))
    {
        display.print(".bad");
    }
    display.print("..ok");
    display.display();
    delay(250);

    display.setCursor(C(0), L(1));
    display.print("WiFi");
    display.display();
    if (!connectWiFi(ssid, password))
    {
        display.print("...bad");
    }
    display.print("....ok");
    display.display();
    delay(250);

    display.setCursor(C(0), L(2));
    auto temperature = getTemperature();
    if (temperature >= 100.)
    {
        display.print("t\xF8.....bad");
    }
    else
    {
        display.print("t\xF8=");
        display.print(temperature, 1);
    }
    display.display();
    delay(250);

    display.setCursor(C(0), L(3));
    display.print("Homing...");
    display.display();
    auto result = window.close();
    display.setCursor(C(0), L(4));
    display.print("Stall=" + String(result));
    display.setCursor(C(0), L(5));
    display.print("Ready!");
    display.display();
    delay(3000);
}

void loop()
{
    auto temperature = getTemperature();

    if (temperature >= 100.)
    {
        display.clearDisplay();
        display.setCursor(C(0), L(0));
        display.print("Can't get");
        display.setCursor(C(0), L(1));
        display.print("temperature");
        display.setCursor(C(0), L(3));
        display.print("err=");
        display.print(temperature, 0);
        display.display();
        delay(500);
        return;
    }

    auto new_position = calculatePosition(temperature);

    display.clearDisplay();
    display.setCursor(C(0), L(0));
    display.print("t\xF8=");
    display.print(temperature, 1);
    display.setCursor(C(0), L(2));
    display.print("new=");
    display.print(new_position);
    display.setCursor(C(0), L(3));
    display.print("curr=");
    display.print(window.getPosition());
    display.display();

    if (abs(new_position - window.getPosition()) > MOVE_THRESHOLD)
    {
        display.setCursor(C(0), L(5));
        display.print("delta=");
        display.print(new_position - window.getPosition());
        display.display();
        window.moveManually(new_position - window.getPosition());
    }

    delay(1000);
}

/// @brief Connect board to WiFi
/// @param ssid WiFi SSID
/// @param password WiFi password
/// @return True if connection was successful, false otherwise
bool connectWiFi(const char *ssid, const char *password)
{
    delay(1000);
    WiFi.begin(ssid, password);
    for (int j = 0; j < 30; ++j)
    {
        delay(1000);
        if (WiFi.status() == WL_CONNECTED)
            return true;
    }
    return false;
}


/// @brief Get temperature from the server
/// @return Temperature
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
            result = 404.; // server does not responding
        http.end();
    }
    else
        result = 408.; // no WiFi connection
    return result;
}

/// @brief Calculate position of the window
/// @param temperature Temperature in the room
/// @return Position of the window in mm
int16_t calculatePosition(float temperature)
{
    if (temperature < MIN_TEMPERATURE)
        return 0;
    if (temperature > MAX_TEMPERATURE)
        return SCREW_LENGTH;

    // map() analog to deal with float
    const float run = MAX_TEMPERATURE - MIN_TEMPERATURE;
    const float delta = temperature - MIN_TEMPERATURE;
    return int16_t(delta / run * SCREW_LENGTH);
}
