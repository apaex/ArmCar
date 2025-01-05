#pragma once

struct UartData
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
