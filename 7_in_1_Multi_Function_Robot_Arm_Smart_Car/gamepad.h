#pragma once

#include "printf.h"
#include "GamepadData.h"

#define GAMEPAD_BUTTON_A 0x0001
#define GAMEPAD_BUTTON_B 0x0002
#define GAMEPAD_BUTTON_X 0x0004
#define GAMEPAD_BUTTON_Y 0x0008

#define GAMEPAD_BUTTON_M1 GAMEPAD_BUTTON_B
#define GAMEPAD_BUTTON_M2 GAMEPAD_BUTTON_A
//#define GAMEPAD_BUTTON_M3 GAMEPAD_BUTTON_Y
#define GAMEPAD_BUTTON_M4 GAMEPAD_BUTTON_X
#define GAMEPAD_BUTTON_M3 GAMEPAD_BUTTON_M1 //нерабочая

#define GAMEPAD_BUTTON_LB 0x0010
#define GAMEPAD_BUTTON_RB 0x0020
#define GAMEPAD_BUTTON_LT 0x0040
#define GAMEPAD_BUTTON_RT 0x0080

#define GAMEPAD_BUTTON_L 0x0100
#define GAMEPAD_BUTTON_R 0x0200

#define GAMEPAD_BUTTON_MISC_SELECT 0x02
#define GAMEPAD_BUTTON_MISC_START 0x04

#define GAMEPAD_BUTTON_DPAD_UP    0x01
#define GAMEPAD_BUTTON_DPAD_DOWN  0x02
#define GAMEPAD_BUTTON_DPAD_RIGHT 0x04
#define GAMEPAD_BUTTON_DPAD_LEFT  0x08


void DebugWrite(const GamepadData& package)
{
  printf(Serial,
      "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, misc: 0x%02x\n",
      package.index,        // Controller Index
      package.dpad,         // D-pad
      package.buttons,      // bitmask of pressed buttons
      package.axisX,        // (-511 - 512) left X Axis
      package.axisY,        // (-511 - 512) left Y axis
      package.axisRX,       // (-511 - 512) right X axis
      package.axisRY,       // (-511 - 512) right Y axis
      package.brake,        // (0 - 1023): brake button
      package.throttle,     // (0 - 1023): throttle (AKA gas) button
      package.miscButtons  // bitmask of pressed "misc" buttons
  );
}
