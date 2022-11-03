#include <Arduino.h>
#include <Window.h>

Window::Window() : TMC2209() 
{
    this->pos = -1;
}

void Window::move(int16_t mm) {
    
}
