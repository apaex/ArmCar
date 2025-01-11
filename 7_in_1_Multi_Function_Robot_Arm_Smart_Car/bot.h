#pragma once
#include "hand.h"
#include "chassis.h"


class Bot
{
public:
    bool trackingSensorLeft = 0;
    bool trackingSensorCenter = 0;
    bool trackingSensorRight = 0;

    bool bumperSensorLeft = 0;
    bool bumperSensorRight = 0;

    float distanceSensor = 0;

    Chassis chassis;
    Hand hand;

    void readTrackerSensors()
    {
        trackingSensorLeft = digitalRead(PIN_TRACKER_LEFT);
        trackingSensorCenter = digitalRead(PIN_TRACKER_CENTER);
        trackingSensorRight = digitalRead(PIN_TRACKER_RIGHT);
    }

    void readBumperSensors()
    {
        bumperSensorLeft = digitalRead(PIN_BUMPER_LEFT);
        bumperSensorRight = digitalRead(PIN_BUMPER_RIGHT);
    }

    void measureDistance()
    {
        digitalWrite(PIN_ULTRASOIC_TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(PIN_ULTRASOIC_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_ULTRASOIC_TRIG, LOW);

        distanceSensor = pulseIn(PIN_ULTRASOIC_ECHO, HIGH) / 58.00;
    }

    void init()
    {
        hand.init();
        chassis.init();
    }

    void readSensors()
    {
        if (chassis.isMoving())
        {
            readTrackerSensors();
            readBumperSensors();
            measureDistance();
        }
    }

    void tick()
    {
        hand.tick();
        chassis.tick();
    }
};