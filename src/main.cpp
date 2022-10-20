// https://wiki.iarduino.ru/page/shagovye-dvigateli/

#include <Arduino.h>
#include <microDS18B20.h>

#define E1 4
#define H1 5
#define E2 6
#define H2 7
#define EN_E 3
#define EN_H 8
#define ENDSTOP 9
#define TEMPERATURE 10
// BUTTON: pin D2, interrupt 0

#define FORWARD 0
#define BACK 1
#define RELEASE 2
#define HOLD 3

#define CLOSE 0
#define OPEN_QUARTER 1
#define OPEN_HALF 2
#define OPEN 3
#define CLOSE_QUARTER 4
#define CLOSE_HALF 5

float delayTime = 1.25;
// float delayTime = 1.15;
bool closed = false;
volatile bool btn_pressed = false;
int state = CLOSE;
MicroDS18B20<TEMPERATURE> ds;

void doStep(bool E1State, bool H1State, bool E2State, bool H2State)
{
  digitalWrite(E1, E1State);
  digitalWrite(H1, H1State);
  digitalWrite(E2, E2State);
  digitalWrite(H2, H2State);
  delay(delayTime);
}

void stepper(byte mode)
{
  switch (mode)
  {
    // case 0: // full step лоховской (full1)
    //   doStep(1, 0, 0, 0);
    //   doStep(0, 1, 0, 0);
    //   doStep(0, 0, 1, 0);
    //   doStep(0, 0, 0, 1);
    //   break;
    // case 1: // full step пацанский (full2)
    //   doStep(1, 0, 0, 1);
    //   doStep(1, 1, 0, 0);
    //   doStep(0, 1, 1, 0);
    //   doStep(0, 0, 1, 1);
    //   break;

  case FORWARD: // half step VIP
    doStep(1, 0, 0, 0);
    doStep(1, 1, 0, 0);
    doStep(0, 1, 0, 0);
    doStep(0, 1, 1, 0);
    doStep(0, 0, 1, 0);
    doStep(0, 0, 1, 1);
    doStep(0, 0, 0, 1);
    doStep(1, 0, 0, 1);
    break;

  case BACK:
    doStep(1, 0, 0, 1);
    doStep(0, 0, 0, 1);
    doStep(0, 0, 1, 1);
    doStep(0, 0, 1, 0);
    doStep(0, 1, 1, 0);
    doStep(0, 1, 0, 0);
    doStep(1, 1, 0, 0);
    doStep(1, 0, 0, 0);
    break;

  case RELEASE:
    doStep(0, 0, 0, 0);
    break;

  case HOLD:
    doStep(1, 1, 0, 0);
    break;
  }
}

void buttonTick()
{
  // в разорванном состоянии - логическая единица
  btn_pressed = true;
  delay(100);
}

float readtemp()
{
  delay(1000);
  float arr[3];
  for (int i = 0; i < 3; ++i)
  {
    ds.requestTemp();
    delay(1000);
    arr[i] = ds.getTemp();
  }
  float k = 0;
  for (int i = 0; i < 3; ++i)
  {
    for (int j = i + 1; j < 3; ++j)
    {
      if (arr[i] > arr[j])
      {
        k = arr[i];
        arr[i] = arr[j];
        arr[j] = k;
      }
    }
  }
  return arr[1];
}

void open(int n)
{
  for (int i = 0; i < n && !btn_pressed; ++i)
  {
    stepper(BACK);
    if (i > (512 * 3 - 10) && !digitalRead(ENDSTOP))
    {
      stepper(RELEASE);
      while (true){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      }
      exit(0);
    }
  }
}

void window(int target)
{
  switch (target)
  {
  case CLOSE:
    while (digitalRead(ENDSTOP) && !btn_pressed)
      stepper(FORWARD);
    break;

  case OPEN_QUARTER:
  case OPEN_HALF:
    open(512 * 3);
    break;

  case OPEN:
    open(512 * 8);
    break;

  case CLOSE_QUARTER:
    for (int i = 0; i < 512 * 8 && !btn_pressed; ++i)
      stepper(FORWARD);
    break;

  case CLOSE_HALF:
    for (int i = 0; i < 512 * 3 && !btn_pressed; ++i)
      stepper(FORWARD);
    break;

  default:
    break;
  }

  stepper(RELEASE);
}

void setup()
{
  pinMode(2, INPUT_PULLUP);
  btn_pressed = false;
  attachInterrupt(0, buttonTick, FALLING);
  if (btn_pressed)
  {
    Serial.begin(9600);
    while (true)
      Serial.println(readtemp());
  }
  for (int i = 3; i < 9; i++)
    pinMode(i, OUTPUT);
  digitalWrite(EN_E, HIGH);
  digitalWrite(EN_H, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ENDSTOP, INPUT_PULLUP);
  readtemp();
  window(CLOSE);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  btn_pressed = false;
}

void loop()
{
  float tmp = readtemp();
  closed = !digitalRead(ENDSTOP);
  if (!btn_pressed)
  {
    if (tmp < 24.5 && !closed)
    {
      state = CLOSE;
      window(CLOSE);
    }
    if (tmp > 25.1 && state == CLOSE)
    {
      state = OPEN_QUARTER;
      window(OPEN_QUARTER);
    }
    if (tmp < 24.9 && state == OPEN_HALF)
    {
      state = OPEN_QUARTER;
      window(CLOSE_QUARTER);
    }
    if (tmp > 25.6 && state == OPEN_QUARTER)
    {
      state = OPEN_HALF;
      window(OPEN_HALF);
    }
    if (tmp < 25.4 && state == OPEN)
    {
      state = OPEN_HALF;
      window(CLOSE_HALF);
    }
    if (tmp > 26.5 && state == OPEN_HALF)
    {
      state = OPEN;
      window(OPEN);
    }
  }
  else
  {
    btn_pressed = false;
    digitalWrite(LED_BUILTIN, HIGH);
    if (closed)
      window(OPEN);
    else
      window(CLOSE);
    //! ждать второго нажатия на кнопку. оно переведет систему обратно в автоматический режим
    while (!btn_pressed)
      delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    btn_pressed = false;
  }
}