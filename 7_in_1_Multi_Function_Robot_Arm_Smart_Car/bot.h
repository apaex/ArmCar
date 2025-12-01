#pragma once
#define GP_MAX_DIST 40ul     // макс. измеряемое расстояние [см]
#include <GyverPing.h>
#include "settings.h"
#include "hand.h"
#include "chassis.h"
#include "debug.h"

class Bot
{
public:
    uint8_t trackingSensorLeft : 1;
    uint8_t trackingSensorCenter : 1;
    uint8_t trackingSensorRight : 1;

    uint8_t bumperSensorLeft : 1;
    uint8_t bumperSensorRight : 1;

    uint8_t distanceSensor = 0;

    bool enableSensors = true;

    Chassis chassis;
    Hand hand;

    Bot() : sonar(PIN_ULTRASOIC_TRIG, PIN_ULTRASOIC_ECHO) {}

    void readTrackerSensors()
    {
        trackingSensorLeft = !digitalRead(PIN_TRACKER_LEFT);
        trackingSensorCenter = !digitalRead(PIN_TRACKER_CENTER);
        trackingSensorRight = !digitalRead(PIN_TRACKER_RIGHT);
    }

    void readBumperSensors()
    {
        bumperSensorLeft = !digitalRead(PIN_BUMPER_LEFT);
        bumperSensorRight = !digitalRead(PIN_BUMPER_RIGHT);
    }

    void measureDistance()
    {
        EVERY(100);

        sonar.ping();
        distanceSensor = sonar.getRaw() / 10;
    }

    void init()
    {
        hand.init();
        chassis.init();
    }

    void readSensors()
    {
        if (enableSensors)
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
        //sonar.tick();
    }

private:
    GPingSync sonar;
};


extern Bot bot;