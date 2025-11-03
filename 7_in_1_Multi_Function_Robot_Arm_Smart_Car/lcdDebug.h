#pragma once
#include <LiquidCrystal_I2C.h>
#include "printf.h"
#include "GamepadData.h"

extern LiquidCrystal_I2C lcd;

template <class T>
void lcd_printAt(uint8_t col, uint8_t row, T param)
{
    lcd.setCursor(col, row);
    lcd.print(param);
}

template <>
void lcd_printAt<uint8_t>(uint8_t col, uint8_t row, uint8_t param)
{
    lcd.setCursor(col, row);
    lcd.print(param, 16);
}


void lcdPrintError(const char* message)
{
  lcd.setCursor(0,0);
  lcd.print(message);
}

void lcdSizeError()
{
  static int count = 0;
  ++count;
  lcd.setCursor(0,1);
  printf(lcd, "Size error: %3d", count);
}

void lcdCrcError()
{
  static int count = 0;
  ++count;
  lcd.setCursor(0,2);
  printf(lcd, "CRC error:  %3d", count);
}


void lcdPacketCount()
{
  static int count = 0;
  ++count;
  lcd.setCursor(0,0);
  printf(lcd, "Count:  %3d", count);
}

void lcdGamepadData(const GamepadData &package)
{
  lcd.setCursor(0,3);
  printf(lcd, "%3d %3d %3d %3d %3d %3d %3d %3d", package.index, package.axisX, package.axisY, package.axisRX, package.axisRY, package.brake, package.throttle, package.buttons);
}

template<class T>
void lcdDebugWrite(const char *st, T v) {
}

template<>
void lcdDebugWrite<int>(const char *st, int v) {

  lcd.setCursor(0,3);
  printf(lcd, "%s %3d     ", st, v);
}

template<>
void lcdDebugWrite<uint16_t>(const char *st, uint16_t v) {

  lcd.setCursor(0,3);
  printf(lcd, "%s %3d     ", st, v);
}

template<>
void lcdDebugWrite<uint32_t>(const char *st, uint32_t v) {

  lcd.setCursor(0,3);
  printf(lcd, "%s %3lu     ", st, v);
}

void lcdShowFps()
{
  static uint32_t nFrames = 0;
  static uint32_t tmr = millis();
  uint32_t now = millis();

  if (now - tmr > 1000)
  {
      uint32_t fps = nFrames * 1000 / (now - tmr);
      tmr = now;
      nFrames = 0;
      lcdDebugWrite("ips", fps);
  }
  ++nFrames;
}

void lcdShowSensors(bool trackingSensorLeft, bool trackingSensorCenter, bool trackingSensorRight, uint8_t distanceSensor, bool bumperSensorLeft, bool bumperSensorRight)
{
/*
  static uint32_t tmr = 0;
  if (millis() - tmr < 100)
    return;
  tmr = millis();
*/

  lcd_printAt(15, 1, trackingSensorLeft ? '^' : 'o');
  lcd_printAt(16, 1, trackingSensorCenter ? '^' : 'o');
  lcd_printAt(17, 1, trackingSensorRight ? '^' : 'o');
  lcd_printAt(14, 2, bumperSensorLeft ? '<' : 'o');
  lcd_printAt(18, 2, bumperSensorRight ? '>' : 'o');
  lcd_printAt(15, 3, "   ");
  lcd_printAt(15, 3, distanceSensor ? distanceSensor : 000);
}