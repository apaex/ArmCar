#pragma once

#include <Servo.h>
#include "settings.h"

struct HandPosition
{
  int base;
  int arm;
  int claw;

  void setBase(int v) {    base = constrain(v, BASE_ANGLE_MIN, BASE_ANGLE_MAX);  }
  void setArm(int v)  {    arm = constrain(v, ARM_ANGLE_MIN, ARM_ANGLE_MAX);  }
  void setClaw(int v) {    claw = constrain(v, CLAW_ANGLE_MIN, CLAW_ANGLE_MAX);  }

  HandPosition() {}
  HandPosition(int base, int arm, int claw)
  {
    setBase(base);
    setArm(arm);
    setClaw(claw);
  }
};

class Hand
{
    Servo servo_base;
    Servo servo_arm;
    Servo servo_claw;

    void setPosition(int base, int arm, int claw)
    {

    }
public:
    void init()
    {    
        servo_base.attach(PIN_SERVO_BASE);
        servo_arm.attach(PIN_SERVO_ARM);
        servo_claw.attach(PIN_SERVO_CLAW);

        setBase(90);
        delay(500);
        setArm(90);
        delay(500);
        setClaw(90);
        delay(500);
    }

    HandPosition current_pos;
    HandPosition target_pos;

    void setBase(int v) {    current_pos.setBase(v);  }    
    void setArm(int v)  {    current_pos.setArm(v);   }
    void setClaw(int v) {    current_pos.setClaw(v);  }

    void incBase(int v) {    setBase(current_pos.base + v); }
    void incArm(int v)  {    setArm(current_pos.arm + v); }
    void incClaw(int v) {    setClaw(current_pos.claw + v); }

    void stop() {}
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
        if (current_pos.base != target_pos.base)
            incBase(current_pos.base < target_pos.base ? 1 : -1);

        if (current_pos.arm != target_pos.arm)
            incArm(current_pos.arm < target_pos.arm ? 1 : -1);

        if (current_pos.claw != target_pos.claw)
            incClaw(current_pos.claw < target_pos.claw ? 1 : -1);

        servo_base.write(current_pos.base); delay(100);
        servo_arm.write(current_pos.arm); delay(100);
        servo_claw.write(current_pos.claw); delay(100);        
    }

    void baseTurnLeft(int speed)   { setBase(BASE_ANGLE_MIN); }
    void baseTurnRight(int speed)  { setBase(BASE_ANGLE_MAX); }
    void armRise(int speed)        { setArm(ARM_ANGLE_MIN); }
    void armDescend(int speed)     { setArm(ARM_ANGLE_MAX); }
    void clawOpen(int speed)       { setClaw(CLAW_ANGLE_MIN); }
    void clawClose(int speed)      { setClaw(CLAW_ANGLE_MAX); }

    void moveTo(HandPosition pos, int speed)         { }
};