#pragma once

const char* debugState(int state)
{
  const static char* states[] = 
  {
    "NONE",
    "STATE_TURNING_LEFT",
    "STATE_TURNING_RIGHT",
    "STATE_MOVING_FORWARD",
    "STATE_MOVING_BACKWARD",
    "STATE_CLAW_CLOSING",
    "STATE_CLAW_OPENING",
    "STATE_ARM_RISING",
    "STATE_ARM_DESCENDING",
    "STATE_BASE_TURNING_LEFT",
    "STATE_BASE_TURNING_RIGHT",
    "MEMORY_ACTION",
    "PROGRAM_AVOIDANCE",
    "PROGRAM_FOLLOWING",
    "PROGRAM_ANTIDROP",
    "PROGRAM_LINE_TRACKING",
  };

  return states[state];
}

#ifdef DEBUG

#include <HardwareSerial.h>

void TimePrint() {
  int time = millis() / 1000;
  if (time / 60 / 60 < 10) { Serial.print("0"); }
  Serial.print(time / 60 / 60);
  Serial.print(":");
  if (time / 60 % 60 < 10) { Serial.print("0"); }
  Serial.print((time / 60) % 60);
  Serial.print(":");
  if (time % 60 < 10) { Serial.print("0"); }
  Serial.print(time % 60);
  Serial.print(" ");
}

template<class T>
void DebugWrite(T v) {
  TimePrint();
  Serial.println(v);
}

template<class T>
void DebugWrite(const char *st, T v) {
  TimePrint();
  Serial.print(st);
  Serial.print(": ");
  Serial.println(v);
}

void DebugWrite(const char *st, int32_t x, int32_t y) {
  TimePrint();
  Serial.print(st);
  Serial.print("(");
  Serial.print(x);
  Serial.print(',');
  Serial.print(y);
  Serial.println(")");
}

template<class T>
void DebugWrite(const char *st, const T arr[], int n) {
  TimePrint();
  Serial.print(st);
  Serial.print(": ");
  
  for (int i = 0; i < n; ++i) {
    Serial.print(arr[i]);
    Serial.print(',');
  }
  Serial.println("");
}

#else

template<class T>
void DebugWrite(T) {}

template<class T>
void DebugWrite(const char *, T) {}

void DebugWrite(const char *, int32_t, int32_t) {}

template<class T>
void DebugWrite(const char *, const T[], int) {}

#endif