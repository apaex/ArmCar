#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"
#include "hand.h"
#include "chassis.h"

IRremote ir(PIN_IR);

#define SPEED_LOW 60
#define SPEED_MEDIUM 120
#define SPEED_HIGH 160

Hand hand;
Chassis chassis;

bool trackingSensorLeft = 0;
bool trackingSensorCenter = 0;
bool trackingSensorRight = 0;

int speed = SPEED_LOW;

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
int nActions = 0;


void ReadTrackerSensors()
{
  trackingSensorLeft = digitalRead(PIN_TRACKER_LEFT);
  trackingSensorCenter = digitalRead(PIN_TRACKER_CENTER);
  trackingSensorRight = digitalRead(PIN_TRACKER_RIGHT);
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
  chassis.moveBackward(speed);
}

void Move_forward_Function()
{
  chassis.moveForward(speed);
}

void Turn_right_Function()
{
  chassis.rotateRight(speed);
}

void Turn_left_Function()
{
  chassis.rotateLeft(speed);
}


void Line_tracking_Function()
{
  if (!trackingSensorLeft && trackingSensorCenter && !trackingSensorRight)
    chassis.moveForward(120);

  else if (trackingSensorLeft && trackingSensorCenter && !trackingSensorRight)
    chassis.rotateLeft(80);
  else if (trackingSensorLeft && !trackingSensorCenter && !trackingSensorRight)
    chassis.rotateLeft(120);

  else if (!trackingSensorLeft && trackingSensorCenter && trackingSensorRight)
    chassis.rotateRight(80);
  else if (!trackingSensorLeft && !trackingSensorCenter && trackingSensorRight)
    chassis.rotateRight(120);

  else if (trackingSensorLeft && trackingSensorCenter && trackingSensorRight)
    chassis.stop();
}

void Following_Function()
{
  int Following_distance = checkdistance();

  if (Following_distance < 15)
    chassis.moveBackward(80);
  else if (Following_distance <= 20)
    chassis.stop();
  else if (Following_distance <= 25)
    chassis.moveForward(80);
  else if (Following_distance <= 30)
    chassis.moveForward(100);
  else
    chassis.stop();
}

void Anti_drop_Function()
{
  if (!trackingSensorLeft && !trackingSensorCenter && !trackingSensorRight)
    chassis.moveForward(60);
  else
  {
    chassis.moveBackward(60);
    delay(600);
    chassis.rotateLeft(60);
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
      chassis.stop();
      delay(100);
      chassis.moveBackward(100);
      delay(600);
    }
    else
    {
      chassis.stop();
      delay(100);
      chassis.rotateLeft(100);
      delay(600);
    }
  }
  else
    chassis.moveForward(70);
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
    chassis.moveForward(100);
    delay(300);
    chassis.stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_DOWN)
  {
    chassis.moveBackward(100);
    delay(300);
    chassis.stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_LEFT)
  {
    chassis.rotateLeft(70);
    delay(300);
    chassis.stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_RIGHT)
  {
    chassis.rotateRight(70);
    delay(300);
    chassis.stop();
  }
  else if (ir.getIrKey(ir.getCode(), 1) == IR_KEYCODE_OK)
  {
    chassis.stop();
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
      case 'S': state = NONE; chassis.stop();      break;
      case 'm': 
        if (nActions < ACTIONS_COUNT)
          mem[nActions++] = hand.position;
        break;    
      case 'a': 
        if (nActions) 
          state = MEMORY_ACTION;
        break;
      case 'X': speed = SPEED_LOW;      break;
      case 'Y': speed = SPEED_MEDIUM;      break;
      case 'Z': speed = SPEED_HIGH;      break;
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

  hand.init();
  hand.setClaw(90);
  delay(500);
  hand.setArm(90);
  delay(500);
  hand.setBase(90);
  delay(500);
  
  chassis.stop();
}

void loop()
{
  IR_control_Function();
  UART_Control();

  ReadTrackerSensors();

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
    case NONE:   ;
  }
}
