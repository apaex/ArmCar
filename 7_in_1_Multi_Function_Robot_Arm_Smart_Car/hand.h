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
        { PIN_SERVO_BASE, BASE_ANGLE_MIN, BASE_ANGLE_MAX, 90 },
        { PIN_SERVO_ARM,  ARM_ANGLE_MIN,  ARM_ANGLE_MAX,  90 },
        { PIN_SERVO_CLAW, CLAW_ANGLE_MIN, CLAW_ANGLE_MAX, 90 },
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
            servos[i].write(current_pos[i]);
            delay(500);
        }
    }

    void baseTurnLeft()   { _target_pos[SERVO_BASE] = BASE_ANGLE_MIN; }
    void baseTurnRight()  { _target_pos[SERVO_BASE] = BASE_ANGLE_MAX; }
    void armRise()        { _target_pos[SERVO_ARM] = ARM_ANGLE_MIN; }
    void armDescend()     { _target_pos[SERVO_ARM] = ARM_ANGLE_MAX; }
    void clawOpen()       { _target_pos[SERVO_CLAW] = CLAW_ANGLE_MIN; }
    void clawClose()      { _target_pos[SERVO_CLAW] = CLAW_ANGLE_MAX; }
    void stop()           { _target_pos = current_pos; }

    void moveTo(HandPosition position)  { _target_pos = position; }

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

            servos[i].write(current_pos[i]);
            delay(100);
        }
    }


};