#include <Arduino.h>

#define BUTTON 21

int ledToggle;
int previousState = HIGH;
unsigned int previousPress;
volatile int buttonFlag;
int buttonDebounce = 500;

void button_ISR()
{
    buttonFlag = 1;
}

void setup()
{
    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON), button_ISR, FALLING);
    Serial.begin(112500);
}

void loop()
{
    if ((millis() - previousPress) > buttonDebounce && buttonFlag)
    {
        previousPress = millis();
        if (digitalRead(BUTTON) == LOW && previousState == HIGH)
            Serial.println('a');

        else if (digitalRead(BUTTON) == HIGH && previousState == LOW)
        {
            previousState = HIGH;
        }
        buttonFlag = 0;
    }
}
