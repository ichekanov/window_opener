#include <Arduino.h>
#include <TMC2209.h>

class Window : private TMC2209
{
public:
    using TMC2209::disableInverseMotorDirection;
    using TMC2209::disableStealthChop;
    using TMC2209::enable;
    using TMC2209::enableAutomaticCurrentScaling;
    using TMC2209::enableInverseMotorDirection;
    using TMC2209::isSetupAndCommunicating;
    using TMC2209::moveAtVelocity;
    using TMC2209::setRunCurrent;
    using TMC2209::setup;

    Window();
    void move(int16_t mm);
    void close();

private:
    int16_t pos;
};
