#pragma once

#include <Servo.h>
#include "settings.h"

struct HandPosition
{
  int claw;
  int arm;
  int base;

  void setClaw(int v) {    claw = constrain(v, 50, 180);  }
  void setArm(int v)  {    arm = constrain(v, 0, 180);  }
  void setBase(int v) {    base = constrain(v, 0, 180);  }

  HandPosition() {}
  HandPosition(int claw, int arm, int base)
  {
    setClaw(claw);
    setArm(arm);
    setBase(base);
  }
};

class Hand
{
    Servo servo_claw;
    Servo servo_arm;
    Servo servo_base;

public:
    void init()
    {    
        servo_claw.attach(PIN_SERVO_CLAW);
        servo_arm.attach(PIN_SERVO_ARM);
        servo_base.attach(PIN_SERVO_BASE);

        setClaw(90);
        delay(500);
        setArm(90);
        delay(500);
        setBase(90);
        delay(500);
    }

    HandPosition position;

    void setClaw(int v) {    position.setClaw(v); servo_claw.write(position.claw); delay(100); }
    void setArm(int v)  {    position.setArm(v);  servo_arm.write(position.arm); delay(100); }
    void setBase(int v) {    position.setBase(v); servo_base.write(position.base); delay(100); }    

    void incClaw(int v) {    setClaw(position.claw + v); }
    void incArm(int v)  {    setArm(position.arm + v); }
    void incBase(int v) {    setBase(position.base + v); }
};