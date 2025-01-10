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
int currentAction = -1;
bool programMayBeRewrite = true;

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
    //push(MOVE_BACKWARD, 60, 200);
    //push(ROTATE_LEFT, 60, 500);

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
  if (!hand.isReady())
    return;

  if (++currentAction >= nActions)
    currentAction = 0;

  hand.moveTo(mem[currentAction]);  
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

  //DebugWrite("vx-rz", vx, rz);

  if (abs(axisY) < GAMEPAD_DEAD_ZONE_Y)
      axisY = 0;
  if (abs(axisRX) < GAMEPAD_DEAD_ZONE_RX)
      axisRX = 0;

  axisY = map(axisY, GAMEPAD_Y_MIN, GAMEPAD_Y_MAX, -255, 255);
  axisRX = map(axisRX, GAMEPAD_X_MIN, GAMEPAD_X_MAX, -255, 255);


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

    chassis.setVelocities(vx, rz, true);
  }
  else if (gamepadMode == 1)
  {
    int r_base = -axisRX;
    int r_arm = axisY;    

    hand.baseTurn(r_base);
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
      case 'x': hand.moveToDefault(); break;

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
      gamepadControl(package);
    else
      DebugWrite("ERROR: size of data package=", res);
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

#define FPS_FRAMES_COUNT 25000

void showFps()
{
  static uint32_t nFrames = 0;
  static uint32_t tmr = millis();

  if (nFrames >= FPS_FRAMES_COUNT) 
  {
      float fps = nFrames * 1000. / (millis() - tmr);
      tmr = millis();
      nFrames = 0;
      DebugWrite("fps", fps);
  }
  ++nFrames;
}


void loop()
{
  //showFps();

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
}
