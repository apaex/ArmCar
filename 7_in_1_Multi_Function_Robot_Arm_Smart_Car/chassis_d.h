#pragma once

#include "settings.h"

class Chassis
{
    void setMotorSpeeds(int dutyL, int dutyR)
    {
        digitalWrite(PIN_MOTOR_LEFT_DIRECTION, dutyL >= 0);
        analogWrite(PIN_MOTOR_LEFT_PWM, abs(dutyL));
        digitalWrite(PIN_MOTOR_RIGHT_DIRECTION, dutyR < 0);
        analogWrite(PIN_MOTOR_RIGHT_PWM, abs(dutyR));
    }

public:
    void init()
    {
        stop();
    }

    void moveForward(int speed)  { setMotorSpeeds(speed, speed); }
    void moveBackward(int speed) { setMotorSpeeds(-speed, -speed); }
    void rotateLeft(int speed)   { setMotorSpeeds(speed, -speed); }
    void rotateRight(int speed)  { setMotorSpeeds(-speed, speed); }
    void stop()                  { setMotorSpeeds(0, 0); }

    void tick() {}
};