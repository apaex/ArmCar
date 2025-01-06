#define DEBUG 1
#define DEBUG_TIME_PRINT 0

#define PIN_SERVO_CLAW 9
#define PIN_SERVO_ARM 10
#define PIN_SERVO_BASE 11

#define PIN_IR 3

#define PIN_TRACKER_LEFT 7
#define PIN_TRACKER_CENTER 8
#define PIN_TRACKER_RIGHT A1

#define PIN_MOTOR_LEFT_DIRECTION 2
#define PIN_MOTOR_LEFT_PWM 5
#define PIN_MOTOR_RIGHT_DIRECTION 4
#define PIN_MOTOR_RIGHT_PWM 6

#define PIN_ULTRASOIC_TRIG 12
#define PIN_ULTRASOIC_ECHO 13



#define STICK_X_MIN -508        //LEFT-RIGHT
#define STICK_X_MAX 508
#define STICK_Y_MIN -508        //FORWARD-BACKWARD
#define STICK_Y_MAX 508

#define MOTOR_SMOOTH_SPEED 10      // плавность скорости моторов
#define MOTOR_MIN_DUTY 30                // мин. сигнал, при котором мотор начинает вращение

#define GAMEPAD_STICK_DEAD_ZONE 5


#define SPEED_LOW 40
#define SPEED_MEDIUM 120
#define SPEED_HIGH 255


#define ACTIONS_COUNT 20

#define BASE_ANGLE_MIN 0
#define BASE_ANGLE_MAX 180
#define ARM_ANGLE_MIN 0
#define ARM_ANGLE_MAX 180
#define CLAW_ANGLE_MIN 50
#define CLAW_ANGLE_MAX 180