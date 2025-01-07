#pragma once

#include <Servo.h>
#include "settings.h"

#define N_SERVOS 3

enum
{
    SERVO_BASE,
    SERVO_ARM,
    SERVO_CLAW
};

struct {
    int pin;
    int min;
    int max;
    int def;
} servosMeta[N_SERVOS] =
    {
        { PIN_SERVO_BASE, SERVO_BASE_MIN, SERVO_BASE_MAX, SERVO_BASE_DEF },
        { PIN_SERVO_ARM,  SERVO_ARM_MIN,  SERVO_ARM_MAX,  SERVO_ARM_DEF },
        { PIN_SERVO_CLAW, SERVO_CLAW_MIN, SERVO_CLAW_MAX, SERVO_CLAW_DEF },
    };

struct HandPosition
{
    int angles[N_SERVOS];

    int& operator[](int index)
    {
        return angles[index];
    }

    HandPosition()
    {}

    HandPosition(const HandPosition& src)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            angles[i] = src.angles[i];
    }

    HandPosition& operator=(const HandPosition& src)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            angles[i] = src.angles[i];
    }
};



class Hand
{
    Servo servos[N_SERVOS];

    HandPosition _target_pos;

    void setMotorAngles(int angles[], bool instantly = false)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            _target_pos[i] = constrain(angles[i], servosMeta[i].min, servosMeta[i].max);

        if (instantly)
        {
            current_pos = _target_pos;
            for (byte i=0; i<N_SERVOS; ++i)
                servos[i].write(current_pos[i]);
        }
    }

public:
    HandPosition current_pos;

    void init()
    {
        for (byte i=0; i<N_SERVOS; ++i)
        {
            servos[i].attach(servosMeta[i].pin);
            _target_pos[i] = current_pos[i] = servosMeta[i].def;
            //servos[i].write(current_pos[i]);
            //delay(500);
        }
    }

    void baseTurnLeft()   { _target_pos[SERVO_BASE] = SERVO_BASE_MIN; }
    void baseTurnRight()  { _target_pos[SERVO_BASE] = SERVO_BASE_MAX; }
    void armRise()        { _target_pos[SERVO_ARM] = SERVO_ARM_MIN; }
    void armDescend()     { _target_pos[SERVO_ARM] = SERVO_ARM_MAX; }
    void clawOpen()       { _target_pos[SERVO_CLAW] = SERVO_CLAW_MIN; }
    void clawClose()      { _target_pos[SERVO_CLAW] = SERVO_CLAW_MAX; }
    void stop()           { _target_pos = current_pos; }

    void moveTo(HandPosition position)  { _target_pos = position; }

    bool isReady() 
    {         
        bool result = true;
        for (byte i=0; i<N_SERVOS; ++i)
            if (current_pos[i] != _target_pos[i])
                result = false;
        return result;    
    }

    void tick()
    {
        static uint32_t tmr;
        if (millis() - tmr < 30)
            return;
        tmr = millis();

        for (byte i=0; i<N_SERVOS; ++i)
        {
            if (current_pos[i] != _target_pos[i])
                current_pos[i] += (current_pos[i] < _target_pos[i] ? 1 : -1);

            //servos[i].write(current_pos[i]);
            //delay(100);
        }
    }


};