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



#define STICK_X_MIN -512        //LEFT-RIGHT
#define STICK_X_MAX 512 //508
#define STICK_Y_MIN -512        //FORWARD-BACKWARD
#define STICK_Y_MAX 512
        
#define MOTOR_SMOOTH_SPEED 10      // плавность скорости моторов
#define MOTOR_MIN_DUTY 0                // мин. сигнал, при котором мотор начинает вращение


#define SPEED_LOW 60
#define SPEED_MEDIUM 120
#define SPEED_HIGH 255


#define ACTIONS_COUNT 20