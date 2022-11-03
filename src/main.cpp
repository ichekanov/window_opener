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
#include <Motor.h>
#include <Wire.h>

int i, n;
int8_t mode = 0;
unsigned long prev_click = 0;

#define OLED_PRINT(s)                                                                                                  \
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
#define VELOCITY 200000
Motor motor;

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

    motor.setup(Serial2);

    I2C_0.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

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
    if (motor.isSetupAndCommunicating())
    {
        OLED_CLEAR_PRINT("TMC ok");
    }
    else
    {
        OLED_CLEAR_PRINT("TMC bad");
        for (;;)
            ;
    }
    display.display();
    motor.enable();
    motor.setRunCurrent(30);
    motor.enableAutomaticCurrentScaling();
    motor.disableInverseMotorDirection();
    // motor.moveAtVelocity(VELOCITY);
    OLED_CLEAR_PRINT("ready!");
}

void loop()
{
    if (!digitalRead(BUTTON) && (millis() - prev_click) > DEBOUNCE_DELAY)
    {
        ++mode;
        if (mode % 2)
        {
            if (mode == 1)
            {
                OLED_CLEAR_PRINT("OPEN");
                motor.disableInverseMotorDirection();
            }
            if (mode == 3)
            {
                OLED_CLEAR_PRINT("CLOSE");
                motor.enableInverseMotorDirection();
            }
            motor.moveAtVelocity(VELOCITY);
        }
        else
        {
            OLED_CLEAR_PRINT("STOP");
            motor.moveAtVelocity(0);
        }
        if (mode == 4)
            mode = 0;
        prev_click = millis();
    }
}
