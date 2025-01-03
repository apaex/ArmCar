#include <Servo.h>
#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"

IRremote ir(3);

String BLE_val = "";
int base_degrees = 90;
int arm_degrees = 90;
int claw_degrees = 90;
boolean forward_flag = false;
boolean backward_flag = false;
boolean left_flag = false;
boolean right_flag = false;
boolean claw_close_flag = false;
boolean claw_open_flag = false;
boolean arm_forward_flag = false;
boolean claw_recracted_flag = false;
boolean base_anticlockwise_flag = false;
boolean base_clockwise_flag = false;
boolean menory_action_flag = false;
boolean Avoidance_Function_flag = false;
boolean Following_Function_flag = false;
boolean Anti_drop_Function_flag = false;
boolean Line_tracking_Function_flag = false;
boolean Gravity_sensor_Function_flag = false;
int Left_Tra_Value = 1;
int Center_Tra_Value = 1;
int Right_Tra_Value = 1;
int Black_Line = 1;
int distance = 0;
int actions_count = 0;
int auto_count = 0;
const int low_speed = 60;
const int medium_speed = 120;
const int high_speed = 160;
int speed_car = 60;

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
  claw_close_flag = true;
  while (claw_close_flag)
  {
    claw_degrees = claw_degrees + 1;
    servo_claw.write(claw_degrees);
    Serial.println(claw_degrees);
    delay(10);
    if (claw_degrees >= 180)
    {
      claw_degrees = 180;
    }
    if (Serial.read() == 's')
    {
      claw_close_flag = false;
    }
  }
}

void claw_open()
{
  claw_close_flag = true;
  while (claw_close_flag)
  {
    claw_degrees = claw_degrees - 1;
    servo_claw.write(claw_degrees);
    Serial.println(claw_degrees);
    delay(10);
    if (claw_degrees <= 50)
    {
      claw_degrees = 50;
    }
    if (Serial.read() == 's')
    {
      claw_close_flag = false;
    }
  }
}

void arm_up()
{
  arm_forward_flag = true;
  while (arm_forward_flag)
  {
    arm_degrees = arm_degrees + 1;
    servo_arm.write(arm_degrees);
    delay(10);
    Serial.println(arm_degrees);
    if (arm_degrees >= 180)
    {
      arm_degrees = 180;
    }
    if (Serial.read() == 's')
    {
      arm_forward_flag = false;
    }
  }
}

void arm_down()
{
  claw_recracted_flag = true;
  while (claw_recracted_flag)
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
    {
      claw_recracted_flag = false;
    }
  }
}


void arm_base_anticlockwise()
{
  base_anticlockwise_flag = true;
  while (base_anticlockwise_flag)
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
    {
      base_anticlockwise_flag = false;
    }
  }
}

void arm_base_clockwise()
{
  base_clockwise_flag = true;
  while (base_clockwise_flag)
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
    {
      base_clockwise_flag = false;
    }
  }
}

void Line_tracking_Function()
{
  Line_tracking_Function_flag = true;
  while (Line_tracking_Function_flag)
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
    if (Serial.read() == 'S')
    {
      Line_tracking_Function_flag = false;
      Stop();
    }
  }
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

void Following_Function()
{
  int Following_distance = 0;
  Following_Function_flag = true;
  while (Following_Function_flag)
  {
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
    if (Serial.read() == 'S')
    {
      Following_Function_flag = false;
      Stop();
    }
  }
}

void ReadTrackerSensors()
{
    Left_Tra_Value = digitalRead(PIN_TRACKER_LEFT);
    Center_Tra_Value = digitalRead(PIN_TRACKER_CENTER);
    Right_Tra_Value = digitalRead(PIN_TRACKER_RIGHT);
}

void Anti_drop_Function()
{
  Anti_drop_Function_flag = true;
  while (Anti_drop_Function_flag)
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
    if (Serial.read() == 'S')
    {
      Anti_drop_Function_flag = false;
      Stop();
    }
  }
}

void Move_backward_Function()
{
  backward_flag = true;
  while (backward_flag)
  {
    Move_Backward(speed_car);
    if (Serial.read() == 'S')
    {
      backward_flag = false;
      Stop();
    }
  }
}

void Move_forward_Function()
{
  forward_flag = true;
  while (forward_flag)
  {
    Move_Forward(speed_car);
    if (Serial.read() == 'S')
    {
      forward_flag = false;
      Stop();
    }
  }
}

void Avoidance_Function()
{
  int Avoidance_distance = 0;
  Avoidance_Function_flag = true;
  while (Avoidance_Function_flag)
  {
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
    if (Serial.read() == 'S')
    {
      Avoidance_Function_flag = false;
      Stop();
    }
  }
}

void Turn_right_Function()
{
  right_flag = true;
  while (right_flag)
  {
    Rotate_Right(speed_car);
    if (Serial.read() == 'S')
    {
      right_flag = false;
      Stop();
    }
  }
}

void Turn_left_Function()
{
  left_flag = true;
  while (left_flag)
  {
    Rotate_Left(speed_car);
    if (Serial.read() == 'S')
    {
      left_flag = false;
      Stop();
    }
  }
}

char bluetooth_val;

void Gravity_sensor_Function()
{
  Gravity_sensor_Function_flag = true;
  while (Gravity_sensor_Function_flag)
  {
    if (Serial.available())
    {
      bluetooth_val = Serial.read();
      Serial.println(bluetooth_val);
      if (bluetooth_val == 'F')
      {
        Move_Forward(speed_car);
      }
      else if (bluetooth_val == 'B')
      {
        Move_Backward(speed_car);
      }
      else if (bluetooth_val == 'L')
      {
        Rotate_Left(speed_car);
      }
      else if (bluetooth_val == 'R')
      {
        Rotate_Right(speed_car);
      }
      else if (bluetooth_val == 'p')
      {
        Stop();
      }
      else if (bluetooth_val == 'X')
      {
        speed_car = low_speed;
      }
      else if (bluetooth_val == 'Y')
      {
        speed_car = medium_speed;
      }
      else if (bluetooth_val == 'Z')
      {
        speed_car = high_speed;
      }
      if (bluetooth_val == 'S')
      {
        Gravity_sensor_Function_flag = false;
        Stop();
      }
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
  Serial.println(auto_count);
  if (0 != auto_count)
  {
    menory_action_flag = true;
  }
  actions_count = 0;
  claw_degrees = servo_claw.read();
  arm_degrees = servo_arm.read();
  base_degrees = servo_base.read();
  while (menory_action_flag)
  {
    for (int i = (1); i <= (auto_count); i = i + (1))
    {
      if (Serial.read() == 's')
      {
        menory_action_flag = false;
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
        menory_action_flag = false;
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
        menory_action_flag = false;
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
        menory_action_flag = false;
        break;
      }
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
    case 'o':
      claw_open();
      break;
    case 'c':
      claw_close();
      break;
    case 'u':
      arm_up();
      break;
    case 'd':
      arm_down();
      break;
    case 'l':
      arm_base_anticlockwise();
      break;
    case 'r':
      arm_base_clockwise();
      break;
    case 'F':
      Move_forward_Function();
      break;
    case 'B':
      Move_backward_Function();
      break;
    case 'L':
      Turn_left_Function();
      break;
    case 'R':
      Turn_right_Function();
      break;
    case 'S':
      Stop();
      break;
    case 'm':
      Serial.println(actions_count);
      read_degress();
      break;
    case 'a':
      auto_do();
      break;
    case 'X':
      speed_car = low_speed;
      break;
    case 'Y':
      speed_car = medium_speed;
      break;
    case 'Z':
      speed_car = high_speed;
      break;
    case 'A':
      Avoidance_Function();
      break;
    case 'D':
      Anti_drop_Function();
      break;
    case 'W':
      Following_Function();
      break;
    case 'T':
      Line_tracking_Function();
      break;
    case 'G':
      Gravity_sensor_Function();
      break;
    }
    BLE_val = "";
  }
  else
  {
    BLE_val = "";
  }
  IR_control_Function();
}
