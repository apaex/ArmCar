#pragma once

#include <Servo.h>
#include "settings.h"
#include "debug.h"

#define N_SERVOS 3

#define SCALE(A) (A << 7)
#define DESCALE(A) (A >> 7)

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
        { PIN_SERVO_BASE, SCALE(SERVO_BASE_MIN), SCALE(SERVO_BASE_MAX), SCALE(SERVO_BASE_DEF) },
        { PIN_SERVO_ARM,  SCALE(SERVO_ARM_MIN),  SCALE(SERVO_ARM_MAX),  SCALE(SERVO_ARM_DEF) },
        { PIN_SERVO_CLAW, SCALE(SERVO_CLAW_MIN), SCALE(SERVO_CLAW_MAX), SCALE(SERVO_CLAW_DEF) },
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
    HandPosition _velocities;

    void setMotorAngles(int angles[], bool instantly = false)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            _target_pos[i] = constrain(angles[i], servosMeta[i].min, servosMeta[i].max);

        if (instantly)
        {
            current_pos = _target_pos;
            for (byte i=0; i<N_SERVOS; ++i)
                servos[i].write(DESCALE(current_pos[i]));
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
            servos[i].write(DESCALE(current_pos[i]));
        }
    }

    void baseTurn(int speed)   { _velocities[SERVO_BASE] = speed; }
    void armRise(int speed)    { _velocities[SERVO_ARM] = speed; }
    void clawOpen(int speed)   { _velocities[SERVO_CLAW] = speed; }

    void setVelocities(int r_base, int r_arm, int r_claw)
    {
        _velocities[SERVO_BASE] = r_base;
        _velocities[SERVO_ARM] = r_arm;
        _velocities[SERVO_CLAW] = r_claw;
    }


    void baseTurnLeft()   { _target_pos[SERVO_BASE] = SCALE(SERVO_BASE_MAX); applyNow(); }
    void baseTurnRight()  { _target_pos[SERVO_BASE] = SCALE(SERVO_BASE_MIN); applyNow(); }
    void armRise()        { _target_pos[SERVO_ARM] = SCALE(SERVO_ARM_MIN);    }
    void armDescend()     { _target_pos[SERVO_ARM] = SCALE(SERVO_ARM_MAX);    }
    void clawOpen()       { _target_pos[SERVO_CLAW] = SCALE(SERVO_CLAW_MIN);  }
    void clawClose()      { _target_pos[SERVO_CLAW] = SCALE(SERVO_CLAW_MAX);  }
    void stop()           { 
        for (byte i=0; i<N_SERVOS; ++i)
            _velocities[i] = 0;
        _target_pos = current_pos; 
    }

    void moveTo(HandPosition position)  { _target_pos = position; }
    void moveToDefault()  
    { 
        for (byte i=0; i<N_SERVOS; ++i)
            _target_pos[i] = servosMeta[i].def;
 
       applyNow();
    }


    void applyNow()
    {
        current_pos = _target_pos;
        for (byte i=0; i<N_SERVOS; ++i)
            servos[i].write(DESCALE(current_pos[i])); 
    }

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
        if (millis() - tmr < SERVO_POLLING_PERIOD)
            return;
        tmr = millis();

//        DebugWrite("current_pos", current_pos.angles, N_SERVOS);
//        DebugWrite("_target_pos", _target_pos.angles, N_SERVOS);
//        DebugWrite("_velocities", _velocities.angles, N_SERVOS);

        for (byte i=0; i<N_SERVOS; ++i)
        {
            if (_velocities[i])
            {
                int pos_old = DESCALE(current_pos[i]);
                current_pos[i] = constrain(current_pos[i] + _velocities[i], servosMeta[i].min, servosMeta[i].max);
                _target_pos[i] = current_pos[i];
                
                int pos_new = DESCALE(current_pos[i]);
                if (pos_old != pos_new)
                    servos[i].write(pos_new);
            }

//            if (current_pos[i] != _target_pos[i])
//                current_pos[i] += (current_pos[i] < _target_pos[i] ? 1 : -1);

        }
    }


};