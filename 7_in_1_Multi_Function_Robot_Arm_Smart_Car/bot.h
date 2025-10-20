#pragma once
#include "settings.h"
#include "hand.h"
#include "chassis.h"
#include "debug.h"
#include "lcd.h"

class Bot
{
public:
    bool trackingSensorLeft = 0;
    bool trackingSensorCenter = 0;
    bool trackingSensorRight = 0;

    bool bumperSensorLeft = 0;
    bool bumperSensorRight = 0;

    uint8_t distanceSensor = 0;

    Chassis chassis;
    Hand hand;

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

    int getMm(uint8_t trig, uint8_t echo, int t)
    {
        // импульс 10 мкс
        digitalWrite(trig, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig, LOW);

        // измеряем время ответного импульса
        uint32_t us = pulseIn(echo, HIGH, 5000);

        // считаем расстояние и возвращаем
        return us * (t * 6 / 10 + 330) / 2000ul;
    }

    void measureDistance()
    {
        static uint32_t tmr;
        if (millis() - tmr < 300)
            return;
        tmr = millis();

        distanceSensor = getMm(PIN_ULTRASOIC_TRIG, PIN_ULTRASOIC_ECHO, 22) / 10;
    }

    void init()
    {
        hand.init();
        chassis.init();
    }

    void readSensors()
    {
        //if (chassis.isMoving())
        {
            bool trackingSensorLeft_old = trackingSensorLeft;
            bool trackingSensorCenter_old = trackingSensorCenter;
            bool trackingSensorRight_old = trackingSensorRight;
            bool bumperSensorLeft_old = bumperSensorLeft;
            bool bumperSensorRight_old = bumperSensorRight;
            uint8_t distanceSensor_old = distanceSensor;

            readTrackerSensors();
            readBumperSensors();
            measureDistance();

            if (
                trackingSensorLeft_old != trackingSensorLeft ||
                trackingSensorCenter_old != trackingSensorCenter ||
                trackingSensorRight_old != trackingSensorRight ||
                bumperSensorLeft_old != bumperSensorLeft ||
                bumperSensorRight_old != bumperSensorRight ||
                abs(distanceSensor_old - distanceSensor) > 1)

                lcdShowSensors(trackingSensorLeft, trackingSensorCenter, trackingSensorRight, distanceSensor, bumperSensorLeft, bumperSensorRight);

                //SerialPrintf("Tracking: %d%d%d, Distance: %d, Bumper: %d%d\n", trackingSensorLeft, trackingSensorCenter, trackingSensorRight, distanceSensor, bumperSensorLeft, bumperSensorRight);
            ;
        }
    }

    void tick()
    {
        hand.tick();
        chassis.tick();
    }
};


extern Bot bot;