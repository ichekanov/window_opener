#include <Arduino.h>
#include <TMC2209.h>

class Window : private TMC2209
{
  public:
    // using TMC2209::disableInverseMotorDirection;
    // using TMC2209::disableStealthChop;
    // using TMC2209::enable;
    // using TMC2209::enableAutomaticCurrentScaling;
    // using TMC2209::enableInverseMotorDirection;
    // using TMC2209::isSetupAndCommunicating;
    // using TMC2209::moveAtVelocity;
    // using TMC2209::setRunCurrent;
    // using TMC2209::setup;
    using TMC2209::getMicrostepsPerStep;

    Window(HardwareSerial &, const int8_t);
    unsigned short move(int16_t);
    unsigned short close();
    void moveManually(int16_t);
    int16_t getPos() const;

  private:
    int16_t pos;    // mm from screw home position.
    int8_t stepPin; // Pin for manual generation of step pulses
};
