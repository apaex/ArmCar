#pragma once

#include <GyverMotor.h>
#include "settings.h"
#include "debug.h"

class Chassis
{
    GMotor motorL;
    GMotor motorR;
    int _dutyL = 0;
    int _dutyR = 0;

    void setMotorSpeeds(int dutyL, int dutyR, bool instantly = false)
    {
        _dutyL = constrain(dutyL, -255, 255);
        _dutyR = constrain(dutyR, -255, 255);
        
        if (instantly)
        {
            motorL.setSpeed(_dutyL);
            motorR.setSpeed(_dutyR);
        }
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
            

    void moveForward(int speed)  { setMotorSpeeds(speed, speed); }
    void moveBackward(int speed) { setMotorSpeeds(-speed, -speed); }
    void rotateLeft(int speed)   { setMotorSpeeds(speed, -speed); }
    void rotateRight(int speed)  { setMotorSpeeds(-speed, speed); }
    void stop()                  { setMotorSpeeds(0, 0); }

    void setFromStickPositions(int dx, int dy, bool instantly = false)
    {
        // преобразуем стики к -255, 255
        int LX = map(dx, STICK_X_MIN, STICK_X_MAX, -255, 255);
        int LY = map(dy, STICK_Y_MIN, STICK_Y_MAX, -255, 255);

        // танковая схема
        int dutyL = LY - LX;
        int dutyR = LY + LX;
        
        setMotorSpeeds(dutyL, dutyR, instantly);
    }
    
    void tick()
    {
        if (motorL._duty != _dutyL)
            motorL.smoothTick(_dutyL);
        if (motorR._duty != _dutyR)
            motorR.smoothTick(_dutyR);
    }
};