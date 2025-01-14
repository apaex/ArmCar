#pragma once
#include <LiquidCrystal_I2C.h>

class LiquidCrystalEx : public LiquidCrystal_I2C
{
    byte x = 0;
    byte y = 0;
public:
    virtual size_t write(uint8_t c)
    {
        if (c == 0x0D)
        {
            x = 0;
            LiquidCrystal_I2C::setCursor(x, y);
        }
        else if (c == 0x0A)
        {
            y++;
            LiquidCrystal_I2C::setCursor(x, y);
        }
        else
        {
            LiquidCrystal_I2C::write(c);
            x++;
        }
        return 1;
    }
};
