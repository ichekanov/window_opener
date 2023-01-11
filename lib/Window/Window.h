#include <Arduino.h>
#include <TMC2209.h>

#define SCREW_LENGTH 120 // mm

class Window : private TMC2209
{
  public:
    Window();
    bool begin(HardwareSerial &, const int8_t);
    unsigned short move(int16_t);
    unsigned short close();
    void moveManually(int16_t);
    int16_t getPosition() const;
    void setPosition(int16_t);

  private:
    int16_t pos;    // mm from screw home position.
    int8_t stepPin; // Pin for manual generation of step pulses
};
