#include <CRC.h>
#include "IR_remote.h"
#include "keymap.h"
#include "settings.h"
#include "hand.h"
#include "chassis.h"
#include "queue.h"
#include "gamepad.h"
#include "bot.h"
#include "debug.h"

IRremote ir(PIN_IR);

Bot bot;

int speed = SPEED_MEDIUM;

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
int currentAction = -1;
bool programMayBeRewrite = true;

void storePosition()
{
  if (nActions < ACTIONS_COUNT)
    mem[nActions++] = bot.hand.current_pos;
}

void Line_tracking_Function()
{
  if (bot.trackingSensorLeft && !bot.trackingSensorCenter && bot.trackingSensorRight)
    bot.chassis.moveForward(120);

  else if (!bot.trackingSensorLeft && bot.trackingSensorRight)
    bot.chassis.rotateLeft(!bot.trackingSensorCenter ? 80 : 120);

  else if (bot.trackingSensorLeft && !bot.trackingSensorRight)
    bot.chassis.rotateRight(!bot.trackingSensorCenter ? 80 : 120);

  else if (!bot.trackingSensorLeft && !bot.trackingSensorCenter && !bot.trackingSensorRight)
    bot.chassis.stop();
}

void Anti_drop_Function()
{
  if (bot.trackingSensorLeft && bot.trackingSensorCenter && bot.trackingSensorRight)
    bot.chassis.moveForward(60);
  else
  {
    //push(MOVE_BACKWARD, 60, 200);
    //push(ROTATE_LEFT, 60, 500);

    bot.chassis.moveBackward(60);
    delay(600);
    bot.chassis.rotateLeft(60);
    delay(500);
  }
}

void Following_Function()
{
  if (bot.distanceSensor < 15)
    bot.chassis.moveBackward(80);
  else if (bot.distanceSensor <= 20)
    bot.chassis.stop();
  else if (bot.distanceSensor <= 25)
    bot.chassis.moveForward(80);
  else if (bot.distanceSensor <= 30)
    bot.chassis.moveForward(100);
  else
    bot.chassis.stop();
}

void Avoidance_Function()
{
  if (bot.distanceSensor <= 25)
  {
    bot.chassis.stop();
    delay(100);
    if (bot.distanceSensor <= 15)
      bot.chassis.moveBackward(100);
    else
      bot.chassis.rotateLeft(100);
    delay(600);
  }
  else
    bot.chassis.moveForward(70);
}


void auto_do()
{
  if (!bot.hand.isReady())
    return;

  if (++currentAction >= nActions)
    currentAction = 0;

  bot.hand.moveTo(mem[currentAction]);
}


void gamepadControl(const GamepadData &package)
{
  static int8_t gamepadMode = 1;

  if (package.buttons & 1)
    gamepadMode = 0;
  if (package.buttons & 2)
    gamepadMode = 1;

  //DebugWrite(package);

  int axisY = package.axisY;
  int axisRX = package.axisRX;
  int axisT = package.throttle - package.brake;

  //DebugWrite("vx-rz", vx, rz);

  if (abs(axisY) < GAMEPAD_DEAD_ZONE_Y)
      axisY = 0;
  if (abs(axisRX) < GAMEPAD_DEAD_ZONE_RX)
      axisRX = 0;

  axisY = map(axisY, GAMEPAD_Y_MIN, GAMEPAD_Y_MAX, -255, 255);
  axisRX = map(axisRX, GAMEPAD_RX_MIN, GAMEPAD_RX_MAX, -255, 255);
  axisT = map(axisT, GAMEPAD_T_MIN, GAMEPAD_T_MAX, -255, 255);


  if (gamepadMode == 0)
  {
    int vx = -axisY;
    int rz = -axisRX;

    rz /= 1.5;
    if (!(package.buttons & 8))
    {
      vx /= 2;
      rz /= 2;
    }

    bot.chassis.setVelocities(vx, rz, true);
  }
  else if (gamepadMode == 1)
  {
    if (package.buttons & 0x0200)
      bot.hand.moveToDefault();
    else
    {
      int r_base = -axisRX;
      int r_arm = axisY;
      int r_claw = axisT;

      bot.hand.setVelocities(r_base, r_arm, r_claw);
    }
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
    case PRG_MOVING_FORWARD:  bot.chassis.moveForward(speed);  break;
    case PRG_MOVING_BACKWARD: bot.chassis.moveBackward(speed); break;
    case PRG_TURNING_LEFT:    bot.chassis.rotateRight(speed);  break;
    case PRG_TURNING_RIGHT:   bot.chassis.rotateLeft(speed);   break;

    case PRG_CLAW_OPENING:        bot.hand.clawOpen();       break;
    case PRG_CLAW_CLOSING:        bot.hand.clawClose();      break;
    case PRG_ARM_RISING:          bot.hand.armRise();        break;
    case PRG_ARM_DESCENDING:      bot.hand.armDescend();     break;
    case PRG_BASE_TURNING_LEFT:   bot.hand.baseTurnLeft();   break;
    case PRG_BASE_TURNING_RIGHT:  bot.hand.baseTurnRight();  break;

    case PRG_NONE:            bot.chassis.stop(); bot.hand.stop();  break;
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
      case 'x': bot.hand.moveToDefault(); break;

      case 'G':
      case 'S': startProgram(PRG_NONE);                 break;

      case 'm':
        if (programMayBeRewrite)
          nActions = 0;
        storePosition();
        programMayBeRewrite = false;
        break;
      case 'a':
        programMayBeRewrite = true;
        currentAction = 0;
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

void IR_control()
{
  static uint32_t tmr;

  static byte old = IR_KEYCODE_OK;
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
    "x",   //IR_KEYCODE_5,
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

  if (map[code][0])
    commandInterpretator(map[code][0]);

  // test
  if (code == IR_KEYCODE_1)
    bot.hand.baseAngle(180);
  else if (code == IR_KEYCODE_3)
    bot.hand.baseAngle(0);
}


void UART_control()
{
  if (!Serial.available())
    return;

  char ch = Serial.read();

  if (ch == '#')
  {
    GamepadData package;
    size_t res = Serial.readBytes((char*)&package, sizeof(package));
    if (res != sizeof(package))
    {
      DebugWrite("ERROR: size of data package=", res);
      return;
    }
    int8_t crc = -1;
    res = Serial.readBytes((char*)&crc, 1);

    int8_t crc2 = calcCRC8((byte*)&package, sizeof(package));
    if (crc != crc2)
    {
      DebugWrite("ERROR: crc of data package=", crc);
      return;
    }

    gamepadControl(package);
  }
  else
    commandInterpretator(ch);
}

void setup()
{
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(SERIAL_TIMEOUT);
  Serial.println("Start");

  pinMode(PIN_MOTOR_LEFT_DIRECTION, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_PWM, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_DIRECTION, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_PWM, OUTPUT);

  pinMode(PIN_TRACKER_LEFT, INPUT);
  pinMode(PIN_TRACKER_CENTER, INPUT);
  pinMode(PIN_TRACKER_RIGHT, INPUT);
  pinMode(PIN_BUMPER_LEFT, INPUT);
  pinMode(PIN_BUMPER_RIGHT, INPUT);

  pinMode(PIN_ULTRASOIC_TRIG, OUTPUT);
  pinMode(PIN_ULTRASOIC_ECHO, INPUT);

  bot.init();
}

void loop()
{
  //showFps();

  IR_control();
  UART_control();

  bot.readSensors();

  switch (program)
  {
    case PRG_MEMORY_ACTION:       auto_do();                  break;
    case PRG_AVOIDANCE:           Avoidance_Function();       break;
    case PRG_ANTIDROP:            Anti_drop_Function();       break;
    case PRG_FOLLOWING:           Following_Function();       break;
    case PRG_LINE_TRACKING:       Line_tracking_Function();   break;
  }

  bot.tick();
}
