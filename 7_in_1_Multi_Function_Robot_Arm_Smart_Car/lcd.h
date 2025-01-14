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
