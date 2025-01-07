#pragma once

#include "serialPrintf.h"

struct GamepadData
{
  uint8_t index;
  uint8_t dpad;
  uint16_t buttons;
  int16_t axisX;
  int16_t axisY;
  int16_t axisRX;
  int16_t axisRY;
  uint16_t brake;
  uint16_t throttle;
  uint8_t miscButtons;
};


void DebugWrite(const GamepadData& package)
{
  SerialPrintf(
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