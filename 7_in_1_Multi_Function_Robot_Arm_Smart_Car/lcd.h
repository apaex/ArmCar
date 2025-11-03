#pragma once
#include <LiquidCrystal_I2C.h>
#include "printf.h"

extern LiquidCrystal_I2C lcd;

class LcdItem {
public:
  LcdItem(byte x_, byte y_, byte w_, byte h_ = 1) : x(x_), y(y_), w(w_), h(h_), updateNeeded(true), enabled(true) {}

  ~LcdItem() { delete p; }

  template<class T>
  void set(T value)
  {
    if (!sz)
    {
      sz = sizeof(T);
      p = new byte[sz];
    }
    else if (sz != sizeof(T))
      return;

    if (*(T*)p != value)
    {
      *(T*)p = value;
      updateNeeded = true;
    }
  }


  void set(const char* value)
  {
    if (!sz)
    {
      sz = w+1;
      p = new byte[sz];
    }

    if (strcmp((char *)p, value) == 0)
    {
      strcpy((char *)p, value);
      updateNeeded = true;
    }
  }

  template<class T>
  T get()
  {
    if (sz == sizeof(T))
      return *(T*)p;
  }

  void enable(bool b)
  {
    if (b == enabled)
      return;
    enabled = b;
    updateNeeded = true;
  }

  virtual void draw() = 0;

  void clear(char ch = ' ')
  {
    for (byte i = x; i < x + w; ++i)
      for (byte j = y; j < y + h; ++j)
      {
        lcd.setCursor(i, j);
        lcd.print(ch);
      }
  }

  void update(bool force = false)
  {
    if (updateNeeded || force)
      if (enabled && sz)
        draw();
      else
        clear();
    updateNeeded = false;
  }

protected:
  byte x:6;
  byte y:2;
  byte w:6;
  byte h:2;

  void *p = 0;
  size_t sz = 0;

private:
  bool updateNeeded:1;
  bool enabled:1;
};


class LcdBool : public LcdItem
{
public:
  LcdBool(byte x_, byte y_, char ch_on_, char ch_off_) : LcdItem(x_, y_, 1, 1), ch_on(ch_on_), ch_off(ch_off_) {}

  virtual void draw()
  {
    lcd.setCursor(x, y);
    lcd.print((*(bool*)p) ? ch_on : ch_off);
  }

private:
  char ch_on = '^';
  char ch_off = 'o';
};


class LcdInt : public LcdItem
{
public:
  LcdInt(byte x_, byte y_, byte w_) : LcdItem(x_, y_, w_, 1) {}

  virtual void draw()
  {
    char buf[w+1];
    char fmt[] = "%0u\0";
    fmt[1] = w + '0';
    if (sz == 4)
    {
      fmt[2] = 'l';
      fmt[3] = 'u';
    }
    switch (sz)
    {
    case 1: snprintf(buf, w+1, fmt, *(uint8_t*)p); break;
    case 2: snprintf(buf, w+1, fmt, *(uint16_t*)p); break;
    case 4: snprintf(buf, w+1, fmt, *(uint32_t*)p); break;
    };

    lcd.setCursor(x, y);
    lcd.print(buf);
  }
};

class LcdFmt : public LcdItem
{
public:
  LcdFmt(byte x_, byte y_, byte w_, const char* fmt_ = 0) : LcdItem(x_, y_, w_, 1), fmt(fmt_) {}


  virtual void draw()
  {
    char buf[w+1];
    snprintf(buf, w+1, fmt, *(uint8_t*)p);

    lcd.setCursor(x, y);
    lcd.print(buf);
  }

  const char* fmt = 0;
};

class LcdChar : public LcdItem
{
public:
  LcdChar(byte x_, byte y_) : LcdItem(x_, y_, 1, 1) {}

  virtual void draw()
  {
    lcd.setCursor(x, y);
    lcd.print(*(char*)p);
  }
};

class LcdStr : public LcdItem
{
public:
  LcdStr(byte x_, byte y_, byte w_) : LcdItem(x_, y_, w_, 1) {}

  virtual void draw()
  {
    lcd.setCursor(x, y);
    lcd.print(*(char*)p);
  }
};

class Lcd
{
public:
  LcdItem** items;
  int count;

  Lcd(LcdItem* items_[], int count_): items(items_), count(count_)
  {
  }

  void update(bool force = false)
  {
    EVERY(100);
    for (int i = 0; i < count; ++i)
      items[i]->update(force);
  }
};
