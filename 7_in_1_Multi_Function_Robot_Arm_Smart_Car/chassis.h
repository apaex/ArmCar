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

    void Move_Backward(int speed) { SetMotors(0, 0, speed); }
    void Rotate_Left(int speed)   { SetMotors(0, 1, speed); }
    void Rotate_Right(int speed)  { SetMotors(1, 0, speed); }
    void Move_Forward(int speed)  { SetMotors(1, 1, speed); }
    void Stop()                   { SetMotors(0, 0, 0); }
};