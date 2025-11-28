#pragma once
#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#else
#include <Servo.h>
#endif
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

    void setVelocity(int servo, int speed) { _velocities[servo] = speed; _target_pos[servo] = current_pos[servo]; }
    void setTarget(int servo, int angle) { _target_pos[servo] = angle; _velocities[servo] = 0; }

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
        attach();
        moveToDefault(true);
    }

    void attach()
    {
        for (byte i=0; i<N_SERVOS; ++i)
            servos[i].attach(servosMeta[i].pin);
    }

    void detach()
    {
        for (byte i=0; i<N_SERVOS; ++i)
            servos[i].detach();
    }

    bool attached()
    {
        return servos[0].attached();
    }

    void setAngles(int a_base, int a_arm, int a_claw)
    {
        setTarget(SERVO_BASE, SCALE(a_base));
        setTarget(SERVO_ARM, SCALE(a_arm));
        setTarget(SERVO_CLAW, SCALE(a_claw));
    }

    void moveTo(HandPosition position, bool instantly = false)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            setTarget(i, position[i]);

        if (instantly)
            applyNow();
    }

    void moveToDefault(bool instantly = false)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            setTarget(i, servosMeta[i].def);

        if (instantly)
            applyNow();
    }

    void baseTurnLeft(int speed = SERVO_DEFAULT_VELOCITY)   { setVelocity(SERVO_BASE, speed);  }
    void baseTurnRight(int speed = SERVO_DEFAULT_VELOCITY)  { setVelocity(SERVO_BASE, -speed); }
    void armRise(int speed = SERVO_DEFAULT_VELOCITY)        { setVelocity(SERVO_ARM, -speed);  }
    void armDescend(int speed = SERVO_DEFAULT_VELOCITY)     { setVelocity(SERVO_ARM, speed);   }
    void clawOpen(int speed = SERVO_DEFAULT_VELOCITY)       { setVelocity(SERVO_CLAW, -speed); }
    void clawClose(int speed = SERVO_DEFAULT_VELOCITY)      { setVelocity(SERVO_CLAW, speed);  }

    void stop()
    {
        for (byte i=0; i<N_SERVOS; ++i)
            setVelocity(i, 0);
    }

    void setVelocities(int r_base, int r_arm, int r_claw)
    {
        setVelocity(SERVO_BASE, r_base);
        setVelocity(SERVO_ARM, r_arm);
        setVelocity(SERVO_CLAW, r_claw);
    }




    bool isReady()
    {
        bool result = true;
        for (byte i=0; i<N_SERVOS; ++i)
            if (current_pos[i] != _target_pos[i] || _velocities[i])
                result = false;
        return result;
    }

    void tick()
    {
        EVERY(SERVO_POLLING_PERIOD);

//        DebugWrite("current_pos", current_pos.angles, N_SERVOS);
//        DebugWrite("_target_pos", _target_pos.angles, N_SERVOS);
//        DebugWrite("_velocities", _velocities.angles, N_SERVOS);

        for (byte i=0; i<N_SERVOS; ++i)
        {
            int pos_old = current_pos[i];

            if (_velocities[i])
            {
                current_pos[i] = constrain(current_pos[i] + _velocities[i], servosMeta[i].min, servosMeta[i].max);
                _target_pos[i] = current_pos[i];
            }

            if (current_pos[i] - _target_pos[i])
            {
                int v = (current_pos[i] < _target_pos[i] ? SERVO_DEFAULT_VELOCITY : -SERVO_DEFAULT_VELOCITY);
                if (abs(_target_pos[i] - current_pos[i]) < SERVO_DEFAULT_VELOCITY )
                    v = _target_pos[i] - current_pos[i];

                current_pos[i] = constrain(current_pos[i] + v, servosMeta[i].min, servosMeta[i].max);
                _velocities[i] = 0;
            }

            if (pos_old != current_pos[i])
            {
                servos[i].writeMicroseconds(DESCALE(current_pos[i]));
                //DebugWrite("pos", DESCALE(current_pos[i]));
            }
        }
    }

};