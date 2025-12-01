#pragma once
#include <GyverMotor2.h>
#include "settings.h"
#include "debug.h"

enum ChassisState
{
    MOVING_STOP,
    MOVING_FORWARD,
    MOVING_BACKWARD,
    MOVING_LEFT,
    MOVING_RIGHT
};

class Chassis
{
    GMotor2<DRIVER2WIRE_NO_INVERT> motorL;
    GMotor2<DRIVER2WIRE_NO_INVERT> motorR;
    ChassisState _state = MOVING_STOP;

    void setState(int dutyL, int dutyR)
    {
        if (dutyL == dutyR) {
            if (dutyL == 0) _state = MOVING_STOP;
            else if (dutyL > 0) _state = MOVING_FORWARD;
            else if (dutyL < 0) _state = MOVING_BACKWARD;
        }
        else if (dutyL < dutyR) _state = MOVING_LEFT;
        else if (dutyL > dutyR) _state = MOVING_RIGHT;
    }

public:
    Chassis(): motorL(PIN_MOTOR_LEFT_DIRECTION, PIN_MOTOR_LEFT_PWM),
               motorR(PIN_MOTOR_RIGHT_DIRECTION, PIN_MOTOR_RIGHT_PWM)
    {
    }

    void init()
    {
        // мин. сигнал вращения
        motorL.setMinDuty(MOTOR_MIN_DUTY);
        motorR.setMinDuty(MOTOR_MIN_DUTY);

        // плавность скорости моторов
        motorL.smoothMode(false);
        motorR.smoothMode(false);
    }

    void setMotorSpeeds(int dutyL, int dutyR, uint16_t accel = 0)
    {
        dutyL = constrain(dutyL, -255, 255);
        dutyR = constrain(dutyR, -255, 255);

        setState(dutyL, dutyR);

        //DebugWrite("dl-dr", _dutyL, _dutyR);

        if (accel)
        {
            motorL.setSmoothSpeed(accel);
            motorR.setSmoothSpeed(accel);
        }
        motorL.smoothMode(accel);
        motorR.smoothMode(accel);

        motorL.setSpeed(-dutyL);
        motorR.setSpeed(dutyR);
    }

    void setVelocities(int vx, int rz, uint16_t accel = 0)
    {
        // танковая схема
        int dutyL = vx - rz;
        int dutyR = vx + rz;

        return setMotorSpeeds(dutyL, dutyR, accel);
    }

    void moveForward(int speed, uint16_t accel = 0)  { setVelocities(speed, 0, accel); }
    void moveBackward(int speed, uint16_t accel = 0) { setVelocities(-speed, 0, accel); }
    void rotateLeft(int speed, uint16_t accel = 0)   { setVelocities(0, speed, accel); }
    void rotateRight(int speed, uint16_t accel = 0)  { setVelocities(0, -speed, accel); }
    void stop()                  { setMotorSpeeds(0, 0); }

    void tick()
    {
        motorL.tick();
        motorR.tick();
    }

    bool isMoving() const
    {
        return _state != MOVING_STOP;
    }

    ChassisState getState() const { return _state; }
};