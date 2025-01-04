#pragma once

#include <GyverMotor.h>
#include "settings.h"
#include "debug.h"

class Chassis
{
    GMotor motorL;
    GMotor motorR;
    int dutyR = 0;
    int dutyL = 0;

    void setDirection(int dx, int dy)
    {
        // преобразуем стики к -255, 255
        int LX = map(dx*2, STICK_X_MIN, STICK_X_MAX, -255, 255);
        int LY = map(dy*2, STICK_Y_MIN, STICK_Y_MAX, -255, 255);

        // танковая схема
        dutyR = LY + LX;
        dutyL = LY - LX;

        dutyR = constrain(dutyR, -255, 255);
        dutyL = constrain(dutyL, -255, 255);
        
#if MOTOR_SMOOTH_SPEED == 0        
        motorR.setSpeed(dutyR);
        motorL.setSpeed(dutyL);
#endif
        DebugWrite("SetDirection", dx, dy);
    }

public:
    Chassis(): motorL(DRIVER2WIRE_NO_INVERT, PIN_MOTOR_LEFT_DIRECTION, PIN_MOTOR_LEFT_PWM, HIGH),
               motorR(DRIVER2WIRE_NO_INVERT, PIN_MOTOR_RIGHT_DIRECTION, PIN_MOTOR_RIGHT_PWM, HIGH)
    {

    }

    void init()
    {
        motorR.setMode(AUTO);
        motorL.setMode(AUTO);

        // направление гусениц
        motorR.setDirection(REVERSE);
        motorL.setDirection(NORMAL);

        // мин. сигнал вращения
        motorR.setMinDuty(MOTOR_MIN_DUTY);
        motorL.setMinDuty(MOTOR_MIN_DUTY);

        // плавность скорости моторов
        motorR.setSmoothSpeed(MOTOR_SMOOTH_SPEED);
        motorL.setSmoothSpeed(MOTOR_SMOOTH_SPEED);
    }        
            

    void moveForward(int speed)  { setDirection(0, -speed); }
    void moveBackward(int speed) { setDirection(0, speed); }
    void rotateLeft(int speed)   { setDirection(-speed, 0); }
    void rotateRight(int speed)  { setDirection(speed, 0); }
    void stop()                  
    { 
        setDirection(0, 0);
        motorR.setSpeed(0);
        motorL.setSpeed(0);
    }

    void tick()
    {
#if MOTOR_SMOOTH_SPEED > 0              
        motorR.smoothTick(dutyR);
        motorL.smoothTick(dutyL);
#endif
    }
};