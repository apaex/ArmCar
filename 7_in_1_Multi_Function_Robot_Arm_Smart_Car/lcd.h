#pragma once
#include <LiquidCrystal_I2C.h>
#include "printf.h"

extern LiquidCrystal_I2C lcd;

class LcdItemBase {
public:
  LcdItemBase(byte x_, byte y_, byte w_, const char* fmt_) : x(x_), y(y_), w(w_), fmt(fmt_), updateNeeded(true), enabled(true) {}

  ~LcdItemBase() { delete p; }

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
    {
      lcd.setCursor(i, y);
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

  void *p = 0;
  size_t sz = 0;

  const char* fmt = 0;

private:
  bool updateNeeded:1;
  bool enabled:1;
};


template <class T>
class LcdItem : public LcdItemBase
{
public:
  using LcdItemBase::LcdItemBase;

  virtual void draw()
  {
    char buf[w+1];
    snprintf(buf, w+1, fmt, *(T*)p);

    lcd.setCursor(x, y);
    lcd.print(buf);
  }
};

template <>
class LcdItem<bool> : public LcdItemBase
{
public:
  using LcdItemBase::LcdItemBase;

  virtual void draw()
  {
    lcd.setCursor(x, y);
    lcd.print(fmt[*(bool*)p]);
  }
};




class Lcd
{
public:
  LcdItemBase** items;
  int count;

  Lcd(LcdItemBase* items_[], int count_): items(items_), count(count_)
  {
  }

  void update(bool force = false)
  {
    EVERY(100);
    for (int i = 0; i < count; ++i)
      items[i]->update(force);
  }
};
