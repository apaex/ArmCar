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