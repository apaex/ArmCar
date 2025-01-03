#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"
#include "hand.h"

IRremote ir(PIN_IR);

#define SPEED_LOW 60
#define SPEED_MEDIUM 120
#define SPEED_HIGH 160
#define BLACK 1

Hand hand(90, 90, 90);

int Left_Tra_Value = 1;
int Center_Tra_Value = 1;
int Right_Tra_Value = 1;

int speed_car = 60;
int nActions = 0;

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
} state;

HandPosition mem[20];


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
  digitalWrite(PIN_ULTRASOIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRASOIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASOIC_TRIG, LOW);
  float distance = pulseIn(PIN_ULTRASOIC_ECHO, HIGH) / 58.00;
  delay(10);
  return distance;
}

void Move_backward_Function()
{
  Move_Backward(speed_car);
}

void Move_forward_Function()
{
  Move_Forward(speed_car);
}

void Turn_right_Function()
{
  Rotate_Right(speed_car);
}

void Turn_left_Function()
{
  Rotate_Left(speed_car);
}


void Line_tracking_Function()
{
  ReadTrackerSensors();

  if (Left_Tra_Value != BLACK && Center_Tra_Value == BLACK && Right_Tra_Value != BLACK)
    Move_Forward(120);
  else if (Left_Tra_Value == BLACK && Center_Tra_Value == BLACK && Right_Tra_Value != BLACK)
    Rotate_Left(80);
  else if (Left_Tra_Value == BLACK && Center_Tra_Value != BLACK && Right_Tra_Value != BLACK)
    Rotate_Left(120);
  else if (Left_Tra_Value != BLACK && Center_Tra_Value != BLACK && Right_Tra_Value == BLACK)
    Rotate_Right(120);
  else if (Left_Tra_Value != BLACK && Center_Tra_Value == BLACK && Right_Tra_Value == BLACK)
    Rotate_Right(80);
  else if (Left_Tra_Value == BLACK && Center_Tra_Value == BLACK && Right_Tra_Value == BLACK)
    Stop();
}

void Following_Function()
{
  int Following_distance = checkdistance();

  if (Following_distance < 15)
    Move_Backward(80);
  else if (Following_distance <= 20)
    Stop();
  else if (Following_distance <= 25)
    Move_Forward(80);
  else if (Following_distance <= 30)
    Move_Forward(100);
  else
    Stop();
}

void Anti_drop_Function()
{
  ReadTrackerSensors();

  if (Left_Tra_Value != BLACK && Center_Tra_Value != BLACK && Right_Tra_Value != BLACK)
    Move_Forward(60);
  else
  {
    Move_Backward(60);
    delay(600);
    Rotate_Left(60);
    delay(500);
  }
}

void Avoidance_Function()
{
  int Avoidance_distance = checkdistance();

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
    Move_Forward(70);
}

void auto_do()
{
  for (int i = 0; i < nActions; ++i)
  {
    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }

    while (hand.position.claw != mem[i].claw)
      hand.incClaw(hand.position.claw < mem[i].claw ? 1 : -1);

    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }

    while (hand.position.arm != mem[i].arm)
      hand.incArm(hand.position.arm < mem[i].arm ? 1 : -1);

    if (Serial.read() == 's')
    {
      state = NONE;
      break;
    }

    while (hand.position.base != mem[i].base)
      hand.incBase(hand.position.base < mem[i].base ? 1 : -1);
  }
  nActions = 0;
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
    hand.incClaw(5);
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_9)
    hand.incClaw(-5);
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_2)
    hand.incArm(5);
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_8)
    hand.incArm(-5);
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_4)
    hand.incBase(5);
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_6)
    hand.incBase(-5);
}


void UART_Control()
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
      case 'G':
      case 'S': state = NONE; Stop();      break;
      case 'm': 
        if (nActions < ACTIONS_COUNT)
          mem[nActions++] = hand.position;
        break;    
      case 'a': 
        if (nActions) 
          state = MEMORY_ACTION;
        break;
      case 'X': speed_car = SPEED_LOW;      break;
      case 'Y': speed_car = SPEED_MEDIUM;      break;
      case 'Z': speed_car = SPEED_HIGH;      break;
      case 'A': state = PROGRAM_AVOIDANCE;      break;
      case 'D': state = PROGRAM_ANTIDROP;      break;
      case 'W': state = PROGRAM_FOLLOWING;      break;
      case 'T': state = PROGRAM_LINE_TRACKING;      break;
    }
  } 
}

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_MOTOR_LEFT_DIRECTION, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_PWM, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_DIRECTION, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_PWM, OUTPUT);

  pinMode(PIN_TRACKER_LEFT, INPUT);
  pinMode(PIN_TRACKER_CENTER, INPUT);
  pinMode(PIN_TRACKER_RIGHT, INPUT);

  pinMode(PIN_ULTRASOIC_TRIG, OUTPUT);
  pinMode(PIN_ULTRASOIC_ECHO, INPUT);

  hand.setClaw(90);
  delay(500);
  hand.setArm(90);
  delay(500);
  hand.setBase(90);
  delay(500);
  
  Stop();
}

void loop()
{
  switch (state)
  {
    case STATE_CLAW_OPENING: hand.incClaw(1);      break;    
    case STATE_CLAW_CLOSING: hand.incClaw(-1);      break;
    case STATE_ARM_RISING: hand.incArm(1);      break;
    case STATE_ARM_DESCENDING: hand.incArm(-1);      break;
    case STATE_BASE_TURNING_LEFT: hand.incBase(1);      break;
    case STATE_BASE_TURNING_RIGHT: hand.incBase(-1);      break;
    case STATE_MOVING_FORWARD: Move_forward_Function();      break;
    case STATE_MOVING_BACKWARD: Move_backward_Function();      break;
    case STATE_TURNING_LEFT: Turn_left_Function();      break;
    case STATE_TURNING_RIGHT: Turn_right_Function();      break;
    case MEMORY_ACTION: auto_do(); break;
    case PROGRAM_AVOIDANCE: Avoidance_Function();      break;
    case PROGRAM_ANTIDROP: Anti_drop_Function();      break;
    case PROGRAM_FOLLOWING: Following_Function();      break;
    case PROGRAM_LINE_TRACKING: Line_tracking_Function();      break;
    case NONE:   
      IR_control_Function();
      UART_Control();
  }

  if (Serial.read() == 's')
    state = NONE;
}
