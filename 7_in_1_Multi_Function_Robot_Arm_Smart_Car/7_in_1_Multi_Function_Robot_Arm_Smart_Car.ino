#include <Servo.h>
#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"

IRremote ir(3);

#define SPEED_LOW 60;
#define SPEED_MEDIUM 120;
#define SPEED_HIGH 160;

int base_degrees = 90;
int arm_degrees = 90;
int claw_degrees = 90;

int Left_Tra_Value = 1;
int Center_Tra_Value = 1;
int Right_Tra_Value = 1;

const int Black_Line = 1;

int distance = 0;
int actions_count = 0;
int auto_count = 0;
int speed_car = 60;

enum State
{
  NONE,
  STATE_TURNING_LEFT,
  STATE_TURNING_RIGHT,
  STATE_MOVING_FORWARD,
  STATE_MOVING_BACKWARD,
  STATE_CLAW_CLOSING,
  STATE_CLAW_OPENING,
  STATE_ARM_RISING,
  STATE_ARM_DESCENDING,
  STATE_BASE_TURNING_LEFT,
  STATE_BASE_TURNING_RIGHT,
  MEMORY_ACTION,
  PROGRAM_AVOIDANCE,
  PROGRAM_FOLLOWING,
  PROGRAM_ANTIDROP,
  PROGRAM_LINE_TRACKING,
  PROGRAM_GRAVITY_SENSOR
} state;

int claw_read_degress[20] = {0, 0, 0};

int arm_read_degress[20] = {0, 0, 0};

int base_read_degress[20] = {0, 0, 0};

Servo servo_claw;
Servo servo_arm;
Servo servo_base;

void SetMotors(bool l, bool r, int speed)
{
  digitalWrite(PIN_MOTOR_LEFT_DIRECTION, l);
  analogWrite(PIN_MOTOR_LEFT_PWM, speed);
  digitalWrite(PIN_MOTOR_RIGHT_DIRECTION, !r);
  analogWrite(PIN_MOTOR_RIGHT_PWM, speed);
}

void Move_Backward(int speed) { SetMotors(0, 0, speed); }
void Rotate_Left(int speed)   { SetMotors(0, 1, speed); }
void Rotate_Right(int speed)  { SetMotors(1, 0, speed); }
void Move_Forward(int speed)  { SetMotors(1, 1, speed); }
void Stop()                   { SetMotors(0, 0, 0); }

void ReadTrackerSensors()
{
    Left_Tra_Value = digitalRead(PIN_TRACKER_LEFT);
    Center_Tra_Value = digitalRead(PIN_TRACKER_CENTER);
    Right_Tra_Value = digitalRead(PIN_TRACKER_RIGHT);
}

float checkdistance()
{
  digitalWrite(12, LOW);
  delayMicroseconds(2);
  digitalWrite(12, HIGH);
  delayMicroseconds(10);
  digitalWrite(12, LOW);
  float distance = pulseIn(13, HIGH) / 58.00;
  delay(10);
  return distance;
}

void read_degress()
{
  if (actions_count <= 19)
  {
    claw_read_degress[(int)((actions_count + 1) - 1)] = servo_claw.read();
    delay(50);
    arm_read_degress[(int)((actions_count + 1) - 1)] = servo_arm.read();
    delay(50);
    base_read_degress[(int)((actions_count + 1) - 1)] = servo_base.read();
    delay(50);
    actions_count = actions_count + 1;
    auto_count = actions_count;
    Serial.println(auto_count);
  }
}

void claw_close()
{
  claw_degrees = claw_degrees + 1;

  servo_claw.write(claw_degrees);
  delay(10);

  Serial.println(claw_degrees);

  if (claw_degrees >= 180)
    claw_degrees = 180;

  if (Serial.read() == 's')
    state = NONE;
}

void claw_open()
{
  claw_degrees = claw_degrees - 1;

  servo_claw.write(claw_degrees);
  delay(10);

  Serial.println(claw_degrees);

  if (claw_degrees <= 50)
    claw_degrees = 50;

  if (Serial.read() == 's')
    state = NONE;
}

void arm_up()
{
  servo_arm.write(arm_degrees);
  delay(10);
  Serial.println(arm_degrees);
  if (arm_degrees >= 180)
  {
    arm_degrees = 180;
  }

  if (Serial.read() == 's')
    state = NONE;
}

void arm_down()
{
  arm_degrees = arm_degrees - 1;
  servo_arm.write(arm_degrees);
  Serial.println(arm_degrees);
  delay(10);
  if (arm_degrees <= 0)
  {
    arm_degrees = 0;
  }

  if (Serial.read() == 's')
    state = NONE;
}


void arm_base_anticlockwise()
{
  base_degrees = base_degrees + 1;
  servo_base.write(base_degrees);
  Serial.println(base_degrees);
  delay(10);
  if (base_degrees >= 180)
  {
    base_degrees = 180;
  }

  if (Serial.read() == 's')
    state = NONE;
}

void arm_base_clockwise()
{
  base_degrees = base_degrees - 1;
  servo_base.write(base_degrees);
  Serial.println(base_degrees);
  delay(10);
  if (base_degrees <= 0)
  {
    base_degrees = 0;
  }

  if (Serial.read() == 's')
    state = NONE;
}

void Line_tracking_Function()
{
  ReadTrackerSensors();

  if (Left_Tra_Value != Black_Line && (Center_Tra_Value == Black_Line && Right_Tra_Value != Black_Line))
  {
    Move_Forward(120);
  }
  else if (Left_Tra_Value == Black_Line && (Center_Tra_Value == Black_Line && Right_Tra_Value != Black_Line))
  {
    Rotate_Left(80);
  }
  else if (Left_Tra_Value == Black_Line && (Center_Tra_Value != Black_Line && Right_Tra_Value != Black_Line))
  {
    Rotate_Left(120);
  }
  else if (Left_Tra_Value != Black_Line && (Center_Tra_Value != Black_Line && Right_Tra_Value == Black_Line))
  {
    Rotate_Right(120);
  }
  else if (Left_Tra_Value != Black_Line && (Center_Tra_Value == Black_Line && Right_Tra_Value == Black_Line))
  {
    Rotate_Right(80);
  }
  else if (Left_Tra_Value == Black_Line && (Center_Tra_Value == Black_Line && Right_Tra_Value == Black_Line))
  {
    Stop();
  }

  if (Serial.read() == 's')
    state = NONE;
}


void Following_Function()
{
  int Following_distance = 0;

  Following_distance = checkdistance();
  if (Following_distance < 15)
  {
    Move_Backward(80);
  }
  else if (15 <= Following_distance && Following_distance <= 20)
  {
    Stop();
  }
  else if (20 <= Following_distance && Following_distance <= 25)
  {
    Move_Forward(80);
  }
  else if (25 <= Following_distance && Following_distance <= 30)
  {
    Move_Forward(100);
  }
  else
  {
    Stop();
  }

  if (Serial.read() == 's')
    state = NONE;
}

void Anti_drop_Function()
{
  ReadTrackerSensors();

  if (Left_Tra_Value != Black_Line && (Center_Tra_Value != Black_Line && Right_Tra_Value != Black_Line))
  {
    Move_Forward(60);
  }
  else
  {
    Move_Backward(60);
    delay(600);
    Rotate_Left(60);
    delay(500);
  }

  if (Serial.read() == 's')
    state = NONE;
}

void Move_backward_Function()
{
  Move_Backward(speed_car);

  if (Serial.read() == 's')
    state = NONE;
}

void Move_forward_Function()
{
  Move_Forward(speed_car);

  if (Serial.read() == 's')
    state = NONE;
}

void Avoidance_Function()
{
  int Avoidance_distance = 0;

  Avoidance_distance = checkdistance();
  if (Avoidance_distance <= 25)
  {
    if (Avoidance_distance <= 15)
    {
      Stop();
      delay(100);
      Move_Backward(100);
      delay(600);
    }
    else
    {
      Stop();
      delay(100);
      Rotate_Left(100);
      delay(600);
    }
  }
  else
  {
    Move_Forward(70);
  }

  if (Serial.read() == 's')
    state = NONE;
}

void Turn_right_Function()
{
  Rotate_Right(speed_car);
  if (Serial.read() == 's')
    state = NONE;
}

void Turn_left_Function()
{
  Rotate_Left(speed_car);
  if (Serial.read() == 's')
    state = NONE;
}


void Gravity_sensor_Function()
{
  char bluetooth_val;

  if (Serial.available())
  {
    bluetooth_val = Serial.read();
    Serial.println(bluetooth_val);

    switch (bluetooth_val)
    {
      case 'F': Move_Forward(speed_car); break;
      case 'B': Move_Backward(speed_car); break;
      case 'L': Rotate_Left(speed_car); break;
      case 'R': Rotate_Right(speed_car); break;
      case 'p': Stop(); break;
      case 'X': speed_car = SPEED_LOW; break;
      case 'Y': speed_car = SPEED_MEDIUM; break;
      case 'Z': speed_car = SPEED_HIGH; break;
    }


    if (bluetooth_val == 'S')
    {
      state = NONE;
      Stop();
    }
  } 
}


void IR_control_Function()
{
  if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_UP)
  {
    Move_Forward(100);
    delay(300);
    Stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_DOWN)
  {
    Move_Backward(100);
    delay(300);
    Stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_LEFT)
  {
    Rotate_Left(70);
    delay(300);
    Stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_RIGHT)
  {
    Rotate_Right(70);
    delay(300);
    Stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_OK)
  {
    Stop();
  }
  else if (false)
  {
  }
  else if (false)
  {
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_7)
  {
    claw_degrees = claw_degrees + 5;
    if (claw_degrees >= 180)
    {
      claw_degrees = 180;
    }
    servo_claw.write(claw_degrees);
    delay(2);
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_9)
  {
    claw_degrees = claw_degrees - 5;
    if (claw_degrees <= 50)
    {
      claw_degrees = 50;
    }
    servo_claw.write(claw_degrees);
    delay(2);
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_2)
  {
    arm_degrees = arm_degrees + 5;
    if (arm_degrees >= 180)
    {
      arm_degrees = 180;
    }
    servo_arm.write(arm_degrees);
    delay(2);
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_8)
  {
    arm_degrees = arm_degrees - 5;
    if (arm_degrees <= 0)
    {
      arm_degrees = 0;
    }
    servo_arm.write(arm_degrees);
    delay(2);
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_4)
  {
    base_degrees = base_degrees + 5;
    if (base_degrees >= 180)
    {
      base_degrees = 180;
    }
    servo_base.write(base_degrees);
    delay(2);
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_6)
  {
    base_degrees = base_degrees - 5;
    if (base_degrees <= 0)
    {
      base_degrees = 0;
    }
    servo_base.write(base_degrees);
    delay(2);
  }
}

void auto_do()
{
  for (int i = (1); i <= (auto_count); i = i + (1))
  {
    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }
    if (claw_degrees < claw_read_degress[(int)(i - 1)])
    {
      while (claw_degrees < claw_read_degress[(int)(i - 1)])
      {
        claw_degrees = claw_degrees + 1;
        servo_claw.write(claw_degrees);
        delay(15);
      }
    }
    else
    {
      while (claw_degrees > claw_read_degress[(int)(i - 1)])
      {
        claw_degrees = claw_degrees - 1;
        servo_claw.write(claw_degrees);
        delay(15);
      }
    }
    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }
    if (arm_degrees < arm_read_degress[(int)(i - 1)])
    {
      while (arm_degrees < arm_read_degress[(int)(i - 1)])
      {
        arm_degrees = arm_degrees + 1;
        servo_arm.write(arm_degrees);
        delay(15);
      }
    }
    else
    {
      while (arm_degrees > arm_read_degress[(int)(i - 1)])
      {
        arm_degrees = arm_degrees - 1;
        servo_arm.write(arm_degrees);
        delay(15);
      }
    }
    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }
    if (base_degrees < base_read_degress[(int)(i - 1)])
    {
      while (base_degrees < base_read_degress[(int)(i - 1)])
      {
        base_degrees = base_degrees + 1;
        servo_base.write(base_degrees);
        delay(15);
      }
    }
    else
    {
      while (base_degrees > base_read_degress[(int)(i - 1)])
      {
        base_degrees = base_degrees - 1;
        servo_base.write(base_degrees);
        delay(15);
      }
    }
    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }
  }
  
}




void setup()
{
  Serial.begin(9600);

  IRremote ir(PIN_IR);

  servo_claw.attach(PIN_SERVO_CLAW);
  servo_arm.attach(PIN_SERVO_ARM);
  servo_base.attach(PIN_SERVO_BASE);

  pinMode(PIN_MOTOR_LEFT_DIRECTION, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_PWM, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_DIRECTION, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_PWM, OUTPUT);

  pinMode(PIN_TRACKER_LEFT, INPUT);
  pinMode(PIN_TRACKER_CENTER, INPUT);
  pinMode(PIN_TRACKER_RIGHT, INPUT);

  pinMode(12, OUTPUT);
  pinMode(13, INPUT);

  servo_claw.write(claw_degrees);
  delay(500);
  servo_arm.write(arm_degrees);
  delay(500);
  servo_base.write(base_degrees);
  delay(500);

  Stop();
}

void loop()
{
  String BLE_val = "";

  while (Serial.available() > 0)
  {
    BLE_val = BLE_val + ((char)(Serial.read()));
    delay(2);
  }

  if (0 < String(BLE_val).length() && 2 >= String(BLE_val).length())
  {
    Serial.println(String(BLE_val).length());
    Serial.println(BLE_val);

    switch (String(BLE_val).charAt(0))
    {
      case 'o': state = STATE_CLAW_OPENING;      break;    
      case 'c': state = STATE_CLAW_CLOSING;      break;
      case 'u': state = STATE_ARM_RISING;      break;
      case 'd': state = STATE_ARM_DESCENDING;      break;
      case 'l': state = STATE_BASE_TURNING_LEFT;      break;
      case 'r': state = STATE_BASE_TURNING_RIGHT;      break;
      case 'F': state = STATE_MOVING_FORWARD;      break;
      case 'B': state = STATE_MOVING_BACKWARD;      break;
      case 'L': state = STATE_TURNING_LEFT;      break;
      case 'R': state = STATE_TURNING_RIGHT;      break;
      case 'S': state = NONE; Stop();      break;
      case 'm': Serial.println(actions_count);      read_degress();      break;    
      case 'a': 
        Serial.println(auto_count);
        if (auto_count) 
        {
          state = MEMORY_ACTION;
          actions_count = 0;
          claw_degrees = servo_claw.read();
          arm_degrees = servo_arm.read();
          base_degrees = servo_base.read();
        }
        break;
      case 'X': speed_car = SPEED_LOW;      break;
      case 'Y': speed_car = SPEED_MEDIUM;      break;
      case 'Z': speed_car = SPEED_HIGH;      break;
      case 'A': state = PROGRAM_AVOIDANCE;      break;
      case 'D': state = PROGRAM_ANTIDROP;      break;
      case 'W': state = PROGRAM_FOLLOWING;      break;
      case 'T': state = PROGRAM_LINE_TRACKING;      break;
      case 'G': state = PROGRAM_GRAVITY_SENSOR;      break;
    }
  }
 
  IR_control_Function();

  switch (state)
  {
    case STATE_CLAW_OPENING: claw_open();      break;    
    case STATE_CLAW_CLOSING: claw_close();      break;
    case STATE_ARM_RISING: arm_up();      break;
    case STATE_ARM_DESCENDING: arm_down();      break;
    case STATE_BASE_TURNING_LEFT: arm_base_anticlockwise();      break;
    case STATE_BASE_TURNING_RIGHT: arm_base_clockwise();      break;
    case STATE_MOVING_FORWARD: Move_forward_Function();      break;
    case STATE_MOVING_BACKWARD: Move_backward_Function();      break;
    case STATE_TURNING_LEFT: Turn_left_Function();      break;
    case STATE_TURNING_RIGHT: Turn_right_Function();      break;
    case MEMORY_ACTION: auto_do(); break;
    case PROGRAM_AVOIDANCE: Avoidance_Function();      break;
    case PROGRAM_ANTIDROP: Anti_drop_Function();      break;
    case PROGRAM_FOLLOWING: Following_Function();      break;
    case PROGRAM_LINE_TRACKING: Line_tracking_Function();      break;
    case PROGRAM_GRAVITY_SENSOR: Gravity_sensor_Function();      break;
  }
}
