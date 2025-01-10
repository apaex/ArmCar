#pragma once

#include <Servo.h>
#include "settings.h"
#include "debug.h"

#define N_SERVOS 3

#define MAP(x, in_min, in_max, out_min, out_max) ( int(int32_t(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min) )

#define DEG2MKS(A) ( MAP(A, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH) )
#define MKS2DEG(A) ( MAP(A, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH, 0, 180) )

#define SCALE(A) ( DEG2MKS(A) << SERVO_SCALE_FACTOR)
#define DESCALE(A) ( A >> SERVO_SCALE_FACTOR)


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

    void applyNow()
    {
        current_pos = _target_pos;
        for (byte i=0; i<N_SERVOS; ++i)
            servos[i].writeMicroseconds(DESCALE(current_pos[i])); 
    }

public:
    HandPosition current_pos;

    void init()
    {
        for (byte i=0; i<N_SERVOS; ++i)
        {
            servos[i].attach(servosMeta[i].pin);
            _target_pos[i] = current_pos[i] = servosMeta[i].def;
            //?servos[i].writeMicroseconds(DESCALE(current_pos[i]));
        }
    }

    void baseVelocity(int speed)   { _velocities[SERVO_BASE] = speed; }
    void armVelocity(int speed)    { _velocities[SERVO_ARM] = speed;  }
    void clawVelocity(int speed)   { _velocities[SERVO_CLAW] = speed; }

    void setVelocities(int r_base, int r_arm, int r_claw)
    {
        _velocities[SERVO_BASE] = r_base;
        _velocities[SERVO_ARM] = r_arm;
        _velocities[SERVO_CLAW] = r_claw;
    }

    void baseAngle(int angle)      { _target_pos[SERVO_BASE] = SCALE(angle);  }
    void armAngle(int angle)       { _target_pos[SERVO_ARM] = SCALE(angle);   }
    void clawAngle(int angle)      { _target_pos[SERVO_CLAW] = SCALE(angle);  }

    void setAngles(int a_base, int a_arm, int a_claw)
    {
        _target_pos[SERVO_BASE] = a_base;
        _target_pos[SERVO_ARM] = a_arm;
        _target_pos[SERVO_CLAW] = a_claw;
    }

    void moveTo(HandPosition position, bool instantly = false)  
    { 
        _target_pos = position; 
        if (instantly)
            applyNow();
    }

    void moveToDefault(bool instantly = false)  
    { 
        for (byte i=0; i<N_SERVOS; ++i)
            _target_pos[i] = servosMeta[i].def;
 
        if (instantly)
            applyNow();
    }

    void stop() 
    { 
        for (byte i=0; i<N_SERVOS; ++i)
            _velocities[i] = 0;
        _target_pos = current_pos; 
    }



    void baseTurnLeft()   { _target_pos[SERVO_BASE] = SCALE(SERVO_BASE_MAX);  }
    void baseTurnRight()  { _target_pos[SERVO_BASE] = SCALE(SERVO_BASE_MIN);  }
    void armRise()        { _target_pos[SERVO_ARM] = SCALE(SERVO_ARM_MIN);    }
    void armDescend()     { _target_pos[SERVO_ARM] = SCALE(SERVO_ARM_MAX);    }
    void clawOpen()       { _target_pos[SERVO_CLAW] = SCALE(SERVO_CLAW_MIN);  }
    void clawClose()      { _target_pos[SERVO_CLAW] = SCALE(SERVO_CLAW_MAX);  }




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
            if (current_pos[i] - _target_pos[i])
            {
                int v = (current_pos[i] < _target_pos[i] ? SERVO_DEFAULT_VELOCITY : -SERVO_DEFAULT_VELOCITY);
                if (abs((_target_pos[i] - current_pos[i])) < SERVO_DEFAULT_VELOCITY )
                    v = _target_pos[i] - current_pos[i];

                int pos_old = DESCALE(current_pos[i]);
                current_pos[i] = constrain(current_pos[i] + v, servosMeta[i].min, servosMeta[i].max);
                
                int pos_new = DESCALE(current_pos[i]);
                if (pos_old != pos_new)
                {
                    servos[i].writeMicroseconds(pos_new);
                    //DebugWrite("pos", pos_new);
                }
            }


            if (_velocities[i])
            {
                int pos_old = DESCALE(current_pos[i]);
                current_pos[i] = constrain(current_pos[i] + _velocities[i], servosMeta[i].min, servosMeta[i].max);
                _target_pos[i] = current_pos[i];
                
                int pos_new = DESCALE(current_pos[i]);
                if (pos_old != pos_new)
                {
                    servos[i].writeMicroseconds(pos_new);
                    //DebugWrite("pos", pos_new);
                }
            }
        }
    }

};