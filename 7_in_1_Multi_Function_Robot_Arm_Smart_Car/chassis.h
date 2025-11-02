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

        motorL.smoothMode(false);
        motorR.smoothMode(false);

        // плавность скорости моторов
        motorL.setSmoothSpeed(MOTOR_SMOOTH_SPEED);
        motorR.setSmoothSpeed(MOTOR_SMOOTH_SPEED);
    }

    void setMotorSpeeds(int dutyL, int dutyR, bool instantly = false)
    {
        dutyL = constrain(dutyL, -255, 255);
        dutyR = constrain(dutyR, -255, 255);

        setState(dutyL, dutyR);

        //DebugWrite("dl-dr", _dutyL, _dutyR);
        motorL.smoothMode(!instantly);
        motorR.smoothMode(!instantly);
        motorL.setSpeed(-dutyL);
        motorR.setSpeed(dutyR);
    }

    void setVelocities(int vx, int rz, bool instantly = false)
    {
        // танковая схема
        int dutyL = vx - rz;
        int dutyR = vx + rz;

        return setMotorSpeeds(dutyL, dutyR, instantly);
    }

    void moveForward(int speed)  { setMotorSpeeds(speed, speed); }
    void moveBackward(int speed) { setMotorSpeeds(-speed, -speed); }
    void rotateLeft(int speed)   { setMotorSpeeds(speed, -speed); }
    void rotateRight(int speed)  { setMotorSpeeds(-speed, speed); }
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