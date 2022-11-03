#include <Arduino.h>
#include <Motor.h>

Motor::Motor() : TMC2209() 
{
    this->pos = -1;
}

void Motor::move(int16_t mm) {
    
}
