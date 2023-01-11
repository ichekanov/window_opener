// #include "../lib/test/button_test.cpp"
// #include "../lib/test/driver_connection_test.cpp"
// #include "../lib/test/i2c_scan.cpp"
// #include "../lib/test/interrupt_test.cpp"
// #include "../lib/test/motor_manual_operation_test.cpp"
// #include "../lib/test/oled_test.cpp"
// #include "../lib/test/wifi_test.cpp"
// #include "../lib/test/http_get_test.cpp"
// #include "../lib/test/oled_ui_test.cpp"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <Secrets.h>
#include <WiFi.h>
#include <Window.h>
#include <Wire.h>

// Because of the size of the screen, I decided to initialize 128*64 OLED 
// display, but I can use only 64*48 pixels. So I need an offset for cursor.
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

#define DISPLAY_TIME 10000
long lastMotion = 0;
volatile bool buttonPressed = false;

float getTemperature();
void connectWiFi(const char *, const char *);
int16_t calculatePosition(float);
void showIP();
void moveWindow();
void showWiFiDisconnected();
void showSensorError();
void drawStatus();
void displayStatus(float, int16_t);

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
        display.print(" bad");
        display.setCursor(C(0), L(1));
        display.print("Reboot...");
        display.display();
        delay(3000);
        ESP.restart();
    }
    display.print("  ok");
    display.display();
    delay(250);

    display.setCursor(C(0), L(1));
    display.print("WiFi...");
    display.display();
    connectWiFi(ssid, password);
    delay(2000);

    display.setCursor(C(0), L(2));
    display.print("Homing...");
    display.display();
    auto result = window.close();
    display.setCursor(C(0), L(3));
    display.print("Stall=" + String(result));
    display.setCursor(C(0), L(5));
    display.print("Ready!");
    display.display();

    attachInterrupt(BUTTON, []() {
        buttonPressed = true;
    }, FALLING);

    lastMotion = millis();
    delay(500);
}

void loop()
{
    auto temperature = getTemperature();
    auto newPosition = calculatePosition(temperature);

    if (buttonPressed)
    {
        buttonPressed = false;
        lastMotion = millis();
    }

    if (millis() - lastMotion < DISPLAY_TIME)
        displayStatus(temperature, newPosition);
    else
    {
        display.clearDisplay();
        display.display();
    }

    if (abs(newPosition - window.getPosition()) > MOVE_THRESHOLD)
    {
        window.moveManually(newPosition - window.getPosition());
        lastMotion = millis();
    }

    delay(1000);
}

/// @brief Connect board to WiFi
/// @param ssid WiFi SSID
/// @param password WiFi password
void connectWiFi(const char *ssid, const char *password)
{
    delay(1000);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, password);
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
/// @return Absolute position of the window in mm
int16_t calculatePosition(float temperature)
{
    if (temperature < MIN_TEMPERATURE)
        return 0;
    if (temperature > MAX_TEMPERATURE)
        return SCREW_LENGTH;
    // map() realization to deal with float:
    const float run = MAX_TEMPERATURE - MIN_TEMPERATURE;
    const float delta = temperature - MIN_TEMPERATURE;
    return int16_t(delta / run * SCREW_LENGTH);
}

/// @brief Draw status (percent and progress bar) on the top of the screen
void drawStatus()
{
    auto status = double(window.getPosition()) / double(SCREW_LENGTH) * 100.;
    display.setCursor(C(0), L(0));
    display.print(status, 0);
    display.print("%");
    if (status == 100.)
    {
        display.fillRect(C(4) + 1, L(0) + 1, 64 - 6 * 4 - 1, 6, SSD1306_WHITE);
    }
    else if (status < 10.)
    {
        display.drawRect(C(2) + 1, L(0) + 1, 64 - 6 * 2 - 1, 6, SSD1306_WHITE);
        display.fillRect(C(2) + 1, L(0) + 2, (64 - 6 * 2 - 1) * status / 100., 4, SSD1306_WHITE);
    }
    else
    {
        display.drawRect(C(3) + 1, L(0) + 1, 64 - 6 * 3 - 1, 6, SSD1306_WHITE);
        display.fillRect(C(3) + 1, L(0) + 2, (64 - 6 * 3 - 1) * status / 100., 4, SSD1306_WHITE);
    }
}

/// @brief Display IP address on the screen
void showIP()
{
    display.setCursor(C(0), L(5));
    display.print("IP: ");
    display.print(WiFi.localIP().toString());
    display.display();
    delay(500);
    display.startscrollleft(15, 15);
    delay(2000);
    display.stopscroll();
}

/// @brief Display "Working..." message on the screen
void moveWindow()
{
    display.setCursor(C(0), L(5));
    display.print("Working...");
    display.display();
}

/// @brief Display "No WiFi connection." message on the screen and wait for WiFi reconnection
void showWiFiDisconnected()
{
    display.setCursor(0, L(5));
    display.print("No WiFi connection. ");
    display.display();
    display.startscrollleft(15, 15);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);
    display.stopscroll();
}

/// @brief Display "Sensor error (ERRORCODE)." message on the screen and wait for sensor reconnection
void showSensorError()
{
    display.setCursor(0, L(5));
    display.print("\x13 Sensor error (");
    display.print(getTemperature(), 0);
    display.print(") \x13");
    display.display();
    display.startscrollleft(15, 15);
    while (getTemperature() >= 100.)
        delay(500);
    display.stopscroll();
}

/// @brief Fills the screen with current status
/// @param temperature Temperature in the room
/// @param newPosition New position of the window in mm
void displayStatus(float temperature, int16_t newPosition)
{
    display.clearDisplay();

    drawStatus();

    display.setCursor(C(1), L(1) + 1);
    display.print("t\xF8: ");
    if (temperature >= 100.)
        display.print("error");
    else
        display.print(temperature, 1);

    display.setCursor(C(0), L(3) - 3);
    display.print(MIN_TEMPERATURE, 1);
    display.setCursor(C(0) + 3, L(4) - 3);
    display.print("min");

    display.setCursor(C(6) + 3, L(3) - 3);
    display.print(MAX_TEMPERATURE, 1);
    display.setCursor(C(6) + 6, L(4) - 3);
    display.print("max");

    display.drawFastHLine(0, 54, 128, SSD1306_WHITE);

    if (WiFi.status() != WL_CONNECTED)
        showWiFiDisconnected();
    else if (temperature >= 100.)
        showSensorError();
    else if (abs(newPosition - window.getPosition()) > MOVE_THRESHOLD)
        moveWindow();
    else
        showIP();
}s
