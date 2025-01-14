#pragma once

#include "printf.h"

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

Print* debugStream = &Serial;

void TimePrint() {
#if DEBUG_TIME_PRINT==1
  int time = millis() / 1000;
  if (time / 60 / 60 < 10) { debugStream->print("0"); }
  debugStream->print(time / 60 / 60);
  debugStream->print(":");
  if (time / 60 % 60 < 10) { debugStream->print("0"); }
  debugStream->print((time / 60) % 60);
  debugStream->print(":");
  if (time % 60 < 10) { debugStream->print("0"); }
  debugStream->print(time % 60);
  debugStream->print(" ");
#endif
}

template<class T>
void DebugWrite(T v) {
  TimePrint();
  debugStream->println(v);
}

template<class T>
void DebugWrite(const char *st, T v) {
  TimePrint();
  debugStream->print(st);
  debugStream->print(": ");
  debugStream->println(v);
}

void DebugWrite(const char *st, int32_t x, int32_t y) {
  TimePrint();
  debugStream->print(st);
  debugStream->print("(");
  debugStream->print(x);
  debugStream->print(',');
  debugStream->print(y);
  debugStream->println(")");
}

template<class T>
void DebugWrite(const char *st, const T arr[], int n) {
  TimePrint();
  debugStream->print(st);
  debugStream->print(": ");

  for (int i = 0; i < n; ++i) {
    debugStream->print(arr[i]);
    debugStream->print(',');
  }
  debugStream->println("");
}

#define FPS_FRAMES_COUNT 25000

void showFps()
{
  static uint32_t nFrames = 0;
  static uint32_t tmr = millis();

  if (nFrames >= FPS_FRAMES_COUNT)
  {
      float fps = nFrames * 1000. / (millis() - tmr);
      tmr = millis();
      nFrames = 0;
      DebugWrite("fps", fps);
  }
  ++nFrames;
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