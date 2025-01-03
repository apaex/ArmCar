#pragma once

#include "settings.h"

class Chassis
{
    void SetMotors(bool l, bool r, int speed)
    {
    digitalWrite(PIN_MOTOR_LEFT_DIRECTION, l);
    analogWrite(PIN_MOTOR_LEFT_PWM, speed);
    digitalWrite(PIN_MOTOR_RIGHT_DIRECTION, !r);
    analogWrite(PIN_MOTOR_RIGHT_PWM, speed);
    }

public:
    void init();

    void moveForward(int speed)  { SetMotors(1, 1, speed); }
    void moveBackward(int speed) { SetMotors(0, 0, speed); }
    void rotateLeft(int speed)   { SetMotors(0, 1, speed); }
    void rotateRight(int speed)  { SetMotors(1, 0, speed); }
    void stop()                  { SetMotors(0, 0, 0); }
};