#define DEBUG 0
#define DEBUG_TIME_PRINT 0


#define PIN_SERVO_CLAW 9
#define PIN_SERVO_ARM 10
#define PIN_SERVO_BASE 11

#define PIN_IR 3

#define PIN_TRACKER_LEFT 7
#define PIN_TRACKER_CENTER 8
#define PIN_TRACKER_RIGHT A1

#define PIN_BUMPER_LEFT A3
#define PIN_BUMPER_RIGHT A0

#define PIN_MOTOR_LEFT_DIRECTION 2
#define PIN_MOTOR_LEFT_PWM 5
#define PIN_MOTOR_RIGHT_DIRECTION 4
#define PIN_MOTOR_RIGHT_PWM 6

#define PIN_ULTRASOIC_TRIG 12
#define PIN_ULTRASOIC_ECHO 13



#define MOTOR_SMOOTH_SPEED 100      // плавность скорости моторов
#define MOTOR_MIN_DUTY 30          // мин. сигнал, при котором мотор начинает вращение

#define GAMEPAD_RX_MIN -508        //LEFT-RIGHT
#define GAMEPAD_RX_MAX 508
#define GAMEPAD_RY_MIN -508
#define GAMEPAD_RY_MAX 508
#define GAMEPAD_DEAD_ZONE_RX 9
#define GAMEPAD_DEAD_ZONE_RY 0
#define GAMEPAD_X_MIN -508
#define GAMEPAD_X_MAX 508
#define GAMEPAD_Y_MIN -508         //FORWARD-BACKWARD
#define GAMEPAD_Y_MAX 508
#define GAMEPAD_DEAD_ZONE_X 0
#define GAMEPAD_DEAD_ZONE_Y 0
#define GAMEPAD_T_MIN -1020
#define GAMEPAD_T_MAX 1020


#define SPEED_LOW 40
#define SPEED_MEDIUM 120
#define SPEED_HIGH 255

#define ACTIONS_COUNT 20

#define SERVO_BASE_DEF 90
#define SERVO_ARM_DEF 50
#define SERVO_CLAW_DEF 120

#define SERVO_BASE_MIN 0
#define SERVO_BASE_MAX 180
#define SERVO_ARM_MIN 30    //верх
#define SERVO_ARM_MAX 175   //низ
#define SERVO_CLAW_MIN 120   // открыто
#define SERVO_CLAW_MAX 173  // закрыто

#define SERVO_SCALE_FACTOR 3                                // ((2400-544) << 3)/255 = 58 шагов на самой быстрой скорости [шаг 3 градуса]
#define SERVO_POLLING_PERIOD (48 >> SERVO_SCALE_FACTOR)     // по (48 >> 3) = 6 мс
#define SERVO_DEFAULT_VELOCITY 32

#define BAUD_RATE 9600
#define SERIAL_TIMEOUT 1000

#define DISPLAY_NCOL        20           // размер дисплея: ширина
#define DISPLAY_NROW        4            // размер дисплея: высота
#define DISPLAY_ADDRESS     0x27         // I2C адрес дисплея (0x27 для PCF8574T, 0x3F для PCF8574AT)

#define CHECK_GAMEPAD_CRC   0


#define EVERY(A) ({\
        static uint32_t __tmr = 0; \
        uint32_t __now = millis(); \
        if (__now - __tmr < A) \
            return; \
        __tmr = __now; \
})