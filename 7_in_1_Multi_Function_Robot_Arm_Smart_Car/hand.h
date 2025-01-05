#pragma once

#include <Servo.h>
#include "settings.h"

#define N_SERVOS 3

enum
{
    BASE,
    ARM,
    CLAW
};

struct {
    int min;
    int max;
    int def;
    int pin;
} servosMeta[N_SERVOS] =
    {
        {BASE_ANGLE_MIN, BASE_ANGLE_MAX, 90, PIN_SERVO_BASE},
        {ARM_ANGLE_MIN, ARM_ANGLE_MAX, 90, PIN_SERVO_ARM},
        {CLAW_ANGLE_MIN, CLAW_ANGLE_MAX, 90, PIN_SERVO_CLAW},
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
    
    void setMotorAngles(int angles[], bool instantly = false)
    {
        for (byte i=0; i<N_SERVOS; ++i)
            _target_pos[i] = constrain(angles[i], servosMeta[i].min, servosMeta[i].max);

        if (instantly)
        {
            current_pos = _target_pos;
            for (byte i=0; i<N_SERVOS; ++i)
                servos[i].write(current_pos[i]);
        }
    }

public:
    HandPosition current_pos;

    void init()
    {    
        for (byte i=0; i<N_SERVOS; ++i)
        {
            servos[i].attach(servosMeta[i].pin);
            _target_pos[i] = current_pos[i] = servosMeta[i].def;
            servos[i].write(current_pos[i]);
            delay(500);
        }
    }

    void baseTurnLeft()   { _target_pos[BASE] = BASE_ANGLE_MIN; }
    void baseTurnRight()  { _target_pos[BASE] = BASE_ANGLE_MAX; }
    void armRise()        { _target_pos[ARM] = ARM_ANGLE_MIN; }
    void armDescend()     { _target_pos[ARM] = ARM_ANGLE_MAX; }
    void clawOpen()       { _target_pos[CLAW] = CLAW_ANGLE_MIN; }
    void clawClose()      { _target_pos[CLAW] = CLAW_ANGLE_MAX; }
    void stop()           { _target_pos = current_pos; }

    void moveTo(HandPosition position)  { _target_pos = position; }

    void tick() 
    {
        static uint32_t tmr;
        if (millis() - tmr < 30) 
            return;
        tmr = millis();

        return;
/*            
            // преобразуем стики от 0..255 к -255, 255          

            //if (ps2x.ButtonPressed(PSB_PAD_RIGHT));
            // изменение позиции на величину скорости
            clawP += map(ps2x.Analog(PSS_RX), 0, 255, MAX_ARM_SPEED, -MAX_ARM_SPEED);
            wristP += map(ps2x.Analog(PSS_RY), 0, 255, MAX_ARM_SPEED, -MAX_ARM_SPEED);
            armP += map(ps2x.Analog(PSS_LY), 0, 255, -MAX_ARM_SPEED, MAX_ARM_SPEED);
            yawP += map(ps2x.Analog(PSS_LX), 0, 255, MAX_ARM_SPEED, -MAX_ARM_SPEED);

            // ограничиваем скорость
            clawP = constrain(clawP, CLAW_MIN, CLAW_MAX);
            wristP = constrain(wristP, WRIST_MIN, WRIST_MAX);
            armP = constrain(armP, ARM_MIN, ARM_MAX);
            yawP = constrain(yawP, YAW_MIN, YAW_MAX);

            // переводим в градусы и применяем
            claw.write(clawP / 10);
            wrist.write(wristP / 10);
            arm.write(armP / 10);
            yaw.write(yawP / 10);
*/            


        for (byte i=0; i<N_SERVOS; ++i)
        {
            if (current_pos[i] != _target_pos[i])
                current_pos[i] += (current_pos[i] < _target_pos[i] ? 1 : -1);

            servos[i].write(current_pos[i]);
            delay(100);
        }
    }


};