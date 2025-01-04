#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"
#include "hand.h"
#include "chassis.h"
#include "debug.h"

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

enum Program
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
} program;

HandPosition mem[ACTIONS_COUNT];
int nActions = 0;

void readTrackerSensors()
{
  trackingSensorLeft = digitalRead(PIN_TRACKER_LEFT);
  trackingSensorCenter = digitalRead(PIN_TRACKER_CENTER);
  trackingSensorRight = digitalRead(PIN_TRACKER_RIGHT);
}

float measureDistance()
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

void storePosition()
{
  if (nActions < ACTIONS_COUNT)
    mem[nActions++] = hand.position;
}

void setProgram(Program _program)
{
  if (_program == program)
    return;

  program = _program;
  DebugWrite(debugState(program));

  if (program == NONE)
  {
    chassis.stop();
    hand.stop();
  }
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

  else if (trackingSensorLeft && !trackingSensorRight)
    chassis.rotateLeft(trackingSensorCenter ? 80 : 120);

  else if (!trackingSensorLeft && trackingSensorRight)
    chassis.rotateRight(trackingSensorCenter ? 80 : 120);

  else if (trackingSensorLeft && trackingSensorCenter && trackingSensorRight)
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

void Following_Function()
{
  int dist = measureDistance();

  if (dist < 15)
    chassis.moveBackward(80);
  else if (dist <= 20)
    chassis.stop();
  else if (dist <= 25)
    chassis.moveForward(80);
  else if (dist <= 30)
    chassis.moveForward(100);
  else
    chassis.stop();
}

void Avoidance_Function()
{
  int dist = measureDistance();

  if (dist <= 25)
  {
    chassis.stop();
    delay(100);
    if (dist <= 15)
      chassis.moveBackward(100);
    else
      chassis.rotateLeft(100);
    delay(600);
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
      program = NONE;
      break;
    }

    while (hand.position.claw != mem[i].claw)
      hand.incClaw(hand.position.claw < mem[i].claw ? 1 : -1);

    if (Serial.read() == 's')
    {
      program = NONE;
      break;
    }

    while (hand.position.arm != mem[i].arm)
      hand.incArm(hand.position.arm < mem[i].arm ? 1 : -1);

    if (Serial.read() == 's')
    {
      program = NONE;
      break;
    }

    while (hand.position.base != mem[i].base)
      hand.incBase(hand.position.base < mem[i].base ? 1 : -1);
  }
  nActions = 0;
}

void commandInterpretator(const char* cmd)
{
    DebugWrite("Command", cmd);

    switch (cmd[0])
    {
      case 'o': setProgram(STATE_CLAW_OPENING);         break;
      case 'c': setProgram(STATE_CLAW_CLOSING);         break;
      case 'u': setProgram(STATE_ARM_RISING);           break;
      case 'd': setProgram(STATE_ARM_DESCENDING);       break;
      case 'l': setProgram(STATE_BASE_TURNING_LEFT);    break;
      case 'r': setProgram(STATE_BASE_TURNING_RIGHT);   break;
      case 'F': setProgram(STATE_MOVING_FORWARD);       break;
      case 'B': setProgram(STATE_MOVING_BACKWARD);      break;
      case 'L': setProgram(STATE_TURNING_LEFT);         break;
      case 'R': setProgram(STATE_TURNING_RIGHT);        break;
      case 'G':
      case 'S': setProgram(NONE);                       break;

      case 'm': storePosition(); break;
      case 'a':
        if (nActions)
          setProgram(MEMORY_ACTION);
        break;
      case 'X': speed = SPEED_LOW;      break;
      case 'Y': speed = SPEED_MEDIUM;   break;
      case 'Z': speed = SPEED_HIGH;     break;
      case 'A': setProgram(PROGRAM_AVOIDANCE);      break;
      case 'D': setProgram(PROGRAM_ANTIDROP);       break;
      case 'W': setProgram(PROGRAM_FOLLOWING);      break;
      case 'T': setProgram(PROGRAM_LINE_TRACKING);  break;
    }
}

void IR_control()
{
  byte code = ir.getIrKey(ir.getCode(), 1);
  if (code > 16)
    return;

  static const char* map[] = {
    "",   //IR_KEYCODE_1
    "u",  //IR_KEYCODE_2,
    "",   //IR_KEYCODE_3,
    "l",  //IR_KEYCODE_4,
    "",   //IR_KEYCODE_5,
    "r",  //IR_KEYCODE_6,
    "o",  //IR_KEYCODE_7,
    "d",  //IR_KEYCODE_8,
    "c",  //IR_KEYCODE_9,
    "",   //IR_KEYCODE_0,
    "",   //IR_KEYCODE_STAR,
    "",   //IR_KEYCODE_POUND,
    "F",  //IR_KEYCODE_UP,
    "B",  //IR_KEYCODE_DOWN,
    "S",  //IR_KEYCODE_OK,
    "L",  //IR_KEYCODE_LEFT,
    "R",  //IR_KEYCODE_RIGHT,
  };

  commandInterpretator(map[code]);
}

void UART_control()
{
  String st = "";

  while (Serial.available() > 0)
  {
    st = st + ((char)(Serial.read()));
    delay(2);
  }

  if (0 < String(st).length() && 2 >= String(st).length())
    commandInterpretator(st.c_str());
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
  chassis.init();
}

void loop()
{
  IR_control();
  UART_control();

  readTrackerSensors();

  switch (program)
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
  }
}
