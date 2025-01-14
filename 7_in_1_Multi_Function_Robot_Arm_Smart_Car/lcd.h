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


void lcdPrintError(const char message)
{
  lcd.setCursor(0,0);
  lcd.print(message);
}

void lcdPrintErrorCount(int count)
{
  lcd.setCursor(0,1);
  printf(lcd, "Errors:   %3d", count);
}

void lcdPrintPacketCount(int count)
{
  lcd.setCursor(0,2);
  printf(lcd, "Packages: %3d", count);
}
