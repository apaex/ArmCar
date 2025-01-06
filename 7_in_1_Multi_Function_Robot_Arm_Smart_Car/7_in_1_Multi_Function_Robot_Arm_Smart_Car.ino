#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"
#include "hand.h"
#include "chassis.h"
#include "queue.h"
#include "gamepad.h"
#include "debug.h"

IRremote ir(PIN_IR);

Hand hand;
Chassis chassis;

bool trackingSensorLeft = 0;
bool trackingSensorCenter = 0;
bool trackingSensorRight = 0;

int speed = SPEED_LOW;

enum Program
{
  PRG_NONE,
  PRG_TURNING_LEFT,
  PRG_TURNING_RIGHT,
  PRG_MOVING_FORWARD,
  PRG_MOVING_BACKWARD,
  PRG_CLAW_CLOSING,
  PRG_CLAW_OPENING,
  PRG_ARM_RISING,
  PRG_ARM_DESCENDING,
  PRG_BASE_TURNING_LEFT,
  PRG_BASE_TURNING_RIGHT,
  PRG_MEMORY_ACTION,
  PRG_AVOIDANCE,
  PRG_FOLLOWING,
  PRG_ANTIDROP,
  PRG_LINE_TRACKING,
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
    mem[nActions++] = hand.current_pos;
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
    hand.moveTo(mem[i]);
  }
}



void startProgram(Program _program)
{
  if (_program == program)
    return;

  program = _program;
  DebugWrite(debugState(program));

  switch (program)
  {
    case PRG_MOVING_FORWARD:  chassis.moveForward(speed);  break;
    case PRG_MOVING_BACKWARD: chassis.moveBackward(speed); break;
    case PRG_TURNING_LEFT:    chassis.rotateRight(speed);  break;
    case PRG_TURNING_RIGHT:   chassis.rotateLeft(speed);   break;

    case PRG_CLAW_OPENING:        hand.clawOpen();       break;
    case PRG_CLAW_CLOSING:        hand.clawClose();      break;
    case PRG_ARM_RISING:          hand.armRise();        break;
    case PRG_ARM_DESCENDING:      hand.armDescend();     break;
    case PRG_BASE_TURNING_LEFT:   hand.baseTurnLeft();   break;
    case PRG_BASE_TURNING_RIGHT:  hand.baseTurnRight();  break;

    case PRG_NONE:            chassis.stop(); hand.stop(); break;
  }
}

void commandInterpretator(char cmd)
{
    DebugWrite("Command", cmd);

    switch (cmd)
    {
      case 'F': startProgram(PRG_MOVING_FORWARD);       break;
      case 'B': startProgram(PRG_MOVING_BACKWARD);      break;
      case 'L': startProgram(PRG_TURNING_LEFT);         break;
      case 'R': startProgram(PRG_TURNING_RIGHT);        break;

      case 'o': startProgram(PRG_CLAW_OPENING);         break;
      case 'c': startProgram(PRG_CLAW_CLOSING);         break;
      case 'u': startProgram(PRG_ARM_RISING);           break;
      case 'd': startProgram(PRG_ARM_DESCENDING);       break;
      case 'l': startProgram(PRG_BASE_TURNING_LEFT);    break;
      case 'r': startProgram(PRG_BASE_TURNING_RIGHT);   break;

      case 'G':
      case 'S': startProgram(PRG_NONE);                 break;

      case 'm': storePosition(); break;
      case 'a':
        if (nActions)
          startProgram(PRG_MEMORY_ACTION);
        break;
      case 'X': speed = SPEED_LOW;      break;
      case 'Y': speed = SPEED_MEDIUM;   break;
      case 'Z': speed = SPEED_HIGH;     break;
      case 'A': startProgram(PRG_AVOIDANCE);      break;
      case 'D': startProgram(PRG_ANTIDROP);       break;
      case 'W': startProgram(PRG_FOLLOWING);      break;
      case 'T': startProgram(PRG_LINE_TRACKING);  break;
    }
}

void setFromStickPositions(const GamepadData &package)
{ 
/* 
  SerialPrintf(
      "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
      "misc: 0x%02x\n",
      package.index,        // Controller Index
      package.dpad,         // D-pad
      package.buttons,      // bitmask of pressed buttons
      package.axisX,        // (-511 - 512) left X Axis
      package.axisY,        // (-511 - 512) left Y axis
      package.axisRX,       // (-511 - 512) right X axis
      package.axisRY,       // (-511 - 512) right Y axis
      package.brake,        // (0 - 1023): brake button
      package.throttle,     // (0 - 1023): throttle (AKA gas) button
      package.miscButtons  // bitmask of pressed "misc" buttons
  ); 
*/  
  int vx = package.axisY;
  int rz = package.axisRX;    

  DebugWrite("vx-rz", vx, rz);

  if (abs(vx) < GAMEPAD_STICK_DEAD_ZONE_Y)
      vx = 0;
  if (abs(rz) < GAMEPAD_STICK_DEAD_ZONE_RX)
      rz = 0;

  vx = map(vx, STICK_Y_MAX, STICK_Y_MIN, -255, 255);
  rz = map(rz, STICK_X_MAX, STICK_X_MIN, -255, 255);

  if (!(package.buttons & 8))
  {
    vx >>= 2;
    rz >>= 2;
  }

  chassis.setVelocities(vx, rz, true);
}


void IR_control()
{
  static uint32_t tmr;

  static byte old = -1;
  byte code = ir.getIrKey(ir.getCode(), 1);
  if (code > 16)
  {
    if (millis() - tmr > 200)
      code = IR_KEYCODE_OK;
    else
      return;
  }

  tmr = millis();

  if (old == code)
    return;
  old = code;

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

  commandInterpretator(map[code][0]);
}


void UART_control()
{
  if (!Serial.available())
    return;

  char ch = Serial.read();
  
  if (ch == '#')
  {
    GamepadData package;
    int res = Serial.readBytes((char*)&package, sizeof(package));
    if (res == sizeof(package))
      setFromStickPositions(package);
  }
  else    
    commandInterpretator(ch);
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
    case PRG_MEMORY_ACTION:       auto_do();                  break;
    case PRG_AVOIDANCE:           Avoidance_Function();       break;
    case PRG_ANTIDROP:            Anti_drop_Function();       break;
    case PRG_FOLLOWING:           Following_Function();       break;
    case PRG_LINE_TRACKING:       Line_tracking_Function();   break;
  }

  hand.tick();
  chassis.tick();
  delay(10);
}
