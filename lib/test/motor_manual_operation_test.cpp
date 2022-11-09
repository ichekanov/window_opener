#include <Arduino.h>

#define EN_PIN 7   // enable (CFG6)
#define DIR_PIN 8  // direction
#define STEP_PIN 9 // step
#define BUTTON 3

volatile short mode = 0;

void buttonClicked()
{
    ++mode;
    if (mode == 1)
        digitalWrite(DIR_PIN, LOW);
    if (mode == 3)
        digitalWrite(DIR_PIN, HIGH);
    if (mode == 4)
        mode = 0;
}

void setup()
{
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, HIGH); // deactivate driver (LOW active)

    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, LOW);

    pinMode(STEP_PIN, OUTPUT);
    digitalWrite(STEP_PIN, LOW);

    digitalWrite(EN_PIN, LOW);

    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON), buttonClicked, FALLING);

    // digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
    if ((mode % 2))
        return;
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(200);
}
