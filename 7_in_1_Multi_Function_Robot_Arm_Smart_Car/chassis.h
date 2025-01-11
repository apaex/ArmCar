#pragma once

#include <GyverMotor.h>
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
    GMotor motorL;
    GMotor motorR;
    int _dutyL = 0;
    int _dutyR = 0;
    ChassisState _state = MOVING_STOP;

    void setState(int dutyL, int dutyR)
    {
        if (_dutyL == _dutyR) {
            if (_dutyL == 0) _state = MOVING_STOP;
            else if (_dutyL > 0) _state = MOVING_FORWARD;
            else if (_dutyL < 0) _state = MOVING_BACKWARD;
        }
        else if (_dutyL < _dutyR) _state = MOVING_LEFT;
        else if (_dutyL > _dutyR) _state = MOVING_RIGHT;
    }

public:
    Chassis(): motorL(DRIVER2WIRE_NO_INVERT, PIN_MOTOR_LEFT_DIRECTION, PIN_MOTOR_LEFT_PWM, HIGH),
               motorR(DRIVER2WIRE_NO_INVERT, PIN_MOTOR_RIGHT_DIRECTION, PIN_MOTOR_RIGHT_PWM, HIGH)
    {

    }

    void init()
    {
        motorL.setMode(AUTO);
        motorR.setMode(AUTO);

        // направление гусениц
        motorL.setDirection(REVERSE);
        motorR.setDirection(NORMAL);

        // мин. сигнал вращения
        motorL.setMinDuty(MOTOR_MIN_DUTY);
        motorR.setMinDuty(MOTOR_MIN_DUTY);

        // плавность скорости моторов
        motorL.setSmoothSpeed(MOTOR_SMOOTH_SPEED);
        motorR.setSmoothSpeed(MOTOR_SMOOTH_SPEED);
    }

    void setMotorSpeeds(int dutyL, int dutyR, bool instantly = false)
    {
        _dutyL = constrain(dutyL, -255, 255);
        _dutyR = constrain(dutyR, -255, 255);

        setState(_dutyL, _dutyR);
        //DebugWrite("dL-dR", _dutyL, _dutyR);

        if (instantly)
        {
            motorL.setSpeed(_dutyL);
            motorR.setSpeed(_dutyR);
        }
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
        //DebugWrite("dl-dr", (motorL._duty - _dutyL), (motorR._duty - _dutyR));
        
        if (motorL._duty != _dutyL)
            motorL.smoothTick(_dutyL);
        if (motorR._duty != _dutyR)
            motorR.smoothTick(_dutyR);
    }

    bool isMoving() const
    {
        return _state != MOVING_STOP;
    }
};