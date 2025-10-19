#pragma once
#include <LiquidCrystal_I2C.h>
#include "printf.h"

extern LiquidCrystal_I2C lcd;

template <class T>
void lcd_printAt(uint8_t col, uint8_t row, T param)
{
    lcd.setCursor(col, row);
    lcd.print(param);
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


