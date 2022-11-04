#include <Arduino.h>
#include <Window.h>

#define VELOCITY 600.                      // steps/s
#define VELOCITY_DELAY 1000000. / VELOCITY // us (μs)
#define EMERGENCY_STOP_DELAY 30000         // ms

#define SCREW_PITCH 2.          // mm/rotation
#define STEPS_PER_ROTATION 200. // steps/rotation
#define SCREW_LENGTH 130        // mm

#define STALLGUARD_THRESHOLD 70 // just a number
#define STALLGUARD_DELAY 500    // ms

/// @brief Initialise the Window object.
/// @param serialConn HardwareSerial connection to control TMC2209 by UART.
/// @param stepPin Pin for manual generation of step pulses.
Window::Window(HardwareSerial &serialConn, const int8_t stepPin) : TMC2209()
{
    this->pos = -1;

    this->stepPin = stepPin;
    pinMode(stepPin, OUTPUT);
    digitalWrite(stepPin, LOW);

    this->setup(serialConn);
    if (!(this->isSetupAndCommunicating()))
        exit(1);

    this->enable();
    this->setRunCurrent(40);
    this->enableAutomaticCurrentScaling();
    this->setMicrostepsPerStep(1);
}

/// @brief Controls the motor by passing the speed to the driver's register.
/// @param mm Motion delta.
/// @return STALLGUARD4™ result.
unsigned short Window::move(int16_t mm)
{
    if (mm > SCREW_LENGTH)
        return -1;
    if ((this->pos + mm) < 0)
        return this->close();
    if ((this->pos + mm) > SCREW_LENGTH)
        this->close();
    if (mm < 0)
    {
        mm = abs(mm);
        this->enableInverseMotorDirection();
    }
    else
    {
        this->disableInverseMotorDirection();
    }
    unsigned short currSum = 0;
    unsigned short currN = 0;
    unsigned long lastCount = millis();
    unsigned long motionBegin = millis();
    unsigned short motionTime = STEPS_PER_ROTATION / SCREW_PITCH / VELOCITY * mm;
    this->moveAtVelocity(VELOCITY);
    while (((millis() - motionBegin) < motionTime))
    {
        ++currN;
        currSum += this->getStallGuardResult();
        if ((millis() - lastCount) > STALLGUARD_DELAY)
        {
            if ((currSum / currN) <= STALLGUARD_THRESHOLD)
                break;
            currN = 0;
            currSum = 0;
        }
    }
    this->moveAtVelocity(0);
    if (!currN)
        return 0;
    return currSum / currN;
}

/// @brief Controls the motor with pulses on `stepPin`. This is blocking code! Also, it is unsafe when closing window,
/// since there is no STALLGUARD4™ monitoring.
/// @param mm Motion delta.
void Window::moveManually(int16_t mm)
{
    if (mm > SCREW_LENGTH)
        return;
    if ((this->pos + mm) < 0)
    {
        this->close();
        return;
    }
    if ((this->pos + mm) > SCREW_LENGTH)
        this->close();
    this->moveAtVelocity(0);
    if (mm < 0)
    {
        mm = abs(mm);
        this->enableInverseMotorDirection();
    }
    else
    {
        this->disableInverseMotorDirection();
    }
    int32_t steps = mm * STEPS_PER_ROTATION / SCREW_PITCH;
    for (int32_t i = 0; i < steps; ++i)
    {
        digitalWrite(this->stepPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(this->stepPin, LOW);
        delayMicroseconds(VELOCITY_DELAY);
    }
}

/// @brief Closes the window using STALLGUARD4™.
/// @return STALLGUARD4™ result.
unsigned short Window::close()
{
    unsigned short result;
    for (int i = 0; i < 2; ++i)
    {
        this->moveManually(5);
        result = this->move(INT16_MIN);
    }
    this->pos = 0;
    return result;
}

/// @brief Returns window position.
/// @return mm from screw home position.
int16_t Window::getPos() const
{
    return this->pos;
}
