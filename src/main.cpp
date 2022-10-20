#include <Arduino.h>
#include <microDS18B20.h>

#define EN_E 4
#define E1 5
#define H1 6
#define E2 7
#define H2 8
#define EN_H 9
#define ENDSTOP 10
#define TEMPERATURE 11
#define SWITCH 12
#define LED_BUILTIN 3

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
#define CLOSE_WITH_ENDSTOP 6

#define DELAY_TIME 1.25

bool opened = true;
int state = CLOSE;

MicroDS18B20<TEMPERATURE> ds;

void blink(){
  analogWrite(LED_BUILTIN, 25);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
}

void doStep(bool E1State, bool H1State, bool E2State, bool H2State)
{
  digitalWrite(E1, E1State);
  digitalWrite(H1, H1State);
  digitalWrite(E2, E2State);
  digitalWrite(H2, H2State);
  delay(DELAY_TIME);
}

void stepper(byte mode)
{
  switch (mode)
  {
  case FORWARD:
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
  for (int i = 0; i < n; ++i)
  {
    stepper(BACK);
    if (i > (512 * 4 - 10) && !digitalRead(ENDSTOP))
    {
      stepper(RELEASE);
      while (true)
      {
        analogWrite(LED_BUILTIN, 128);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      }
      exit(0);
    }
  }
}

void window(int target, int current_state)
{
  switch (target)
  {
    case CLOSE_WITH_ENDSTOP:
    {
      for (int i = 0; digitalRead(ENDSTOP) && i < current_state * 5 * 512; ++i)
        stepper(FORWARD);
      // OPEN_QUARTER == 512*3
      // OPEN_HALF == 512*6
      // OPEN == 512*14
      break;
    }

    case CLOSE:
    case CLOSE_QUARTER:
    {      
      for (int i = 0; i < 512 * 3; ++i)
        stepper(FORWARD);
      break;
    }

    case CLOSE_HALF:
    {
      for (int i = 0; i < 512 * 8; ++i)
        stepper(FORWARD);
      break;
    }

    case OPEN_QUARTER:
    case OPEN_HALF:
    {
      open(512 * 3);
      break;
    }

    case OPEN:
    {
      open(512 * 8);
      break;
    }

    default:
      break;
  }

  stepper(RELEASE);
  blink();
}

void smoothLed(int max)
{
  double d = PI / 300;
  double s = 0;
  for (double t = 0; t < 2 * PI; t += d)
  {
    s = max * (sin(t - PI / 2) + 1);
    analogWrite(LED_BUILTIN, s);
    delay(d * 1000);
  }
  delay(200);
}

void setup()
{
  pinMode(SWITCH, INPUT_PULLUP);
  // if (digitalRead(SWITCH))
  // {
  //   Serial.begin(9600);
  //   while (true)
  //     Serial.println(readtemp());
  // }
  for (int i = 4; i < 10; i++)
    pinMode(i, OUTPUT);
  digitalWrite(EN_E, HIGH);
  digitalWrite(EN_H, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(ENDSTOP, INPUT_PULLUP);
  readtemp();
  window(CLOSE_WITH_ENDSTOP, OPEN);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  state = CLOSE;
}

void loop()
{
  if (digitalRead(SWITCH))
  {
    /* manual mode */
    window(CLOSE_WITH_ENDSTOP, state);
    state = CLOSE;
    while (digitalRead(SWITCH))
      smoothLed(20);
    digitalWrite(LED_BUILTIN, LOW);
  }

  float tmp = readtemp();

  if (tmp > 25.1 && state == CLOSE)
  {
    window(OPEN_QUARTER, state);
    state = OPEN_QUARTER;
  }
  if (tmp > 25.6 && state == OPEN_QUARTER)
  {
    window(OPEN_HALF, state);
    state = OPEN_HALF;
  }
  if (tmp > 26.5 && state == OPEN_HALF)
  {
    window(OPEN, state);
    state = OPEN;
  }
  if (tmp < 25.4 && state == OPEN)
  {
    window(CLOSE_HALF, state);
    state = OPEN_HALF;
  }
  if (tmp < 24.9 && state == OPEN_HALF)
  {
    window(CLOSE_QUARTER, state);
    state = OPEN_QUARTER;
  }
  if (tmp < 24.5 && state == OPEN_QUARTER)
  {
    window(CLOSE, state);
    state = CLOSE;
  }

  blink();
  delay(1000);
}
