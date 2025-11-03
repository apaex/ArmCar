#include <CRC.h>
#define NEC_SKIP_REPEATS 0
#include <NecDecoder.h>
#include "settings.h"
#include "hand.h"
#include "chassis.h"
#include "queue.h"
#include "gamepad.h"
#include "bot.h"
#include "debug.h"
#include "lcd.h"
#include "lcdDebug.h"
#include "ir.h"

LiquidCrystal_I2C lcd(DISPLAY_ADDRESS, DISPLAY_NCOL, DISPLAY_NROW);

NecDecoder ir;
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


enum
{
  LCD_FPS,

  LCD_TRACKING_SENSOR_LEFT,
  LCD_TRACKING_SENSOR_CENTER,
  LCD_TRACKING_SENSOR_RIGHT,
  LCD_BUMPER_SENSOR_LEFT,
  LCD_BUMPER_SENSOR_RIGHT,
  LCD_DISTANCE_SENSOR,
  LCD_COMMAND_COUNTER,
  LCD_COMMAND,
  LCD_A1,
  LCD_A2,
  LCD_A3,

  _LCD_COUNT,
};

LcdItem* lcdItems[_LCD_COUNT];
Lcd display(lcdItems, _LCD_COUNT);

uint32_t fps = 0;

void calcFps()
{
  static uint32_t nFrames = 0;
  static uint32_t tmr = millis();
  uint32_t now = millis();

  if (now - tmr > 1000)
  {
      fps = nFrames * 1000 / (now - tmr);
      tmr = now;
      nFrames = 0;
  }
  ++nFrames;
}

void storePosition()
{
  if (nActions < ACTIONS_COUNT)
    mem[nActions++] = bot.hand.current_pos;
}

void Line_tracking_Function()  //ходить по линии
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

void Anti_drop_Function() //не падать со стола
{
  if (bot.trackingSensorLeft && bot.trackingSensorCenter && bot.trackingSensorRight)
    bot.chassis.moveForward(60);
  else
  {
    bot.chassis.moveBackward(60);
    delay(600);
    bot.chassis.rotateLeft(60);
    delay(500);
  }
}

void Following_Function() //преследование
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

void Avoidance_Function() //уклонение
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
  int axisX = (abs(package.axisX) >= GAMEPAD_DEAD_ZONE_X) ? map(package.axisX, GAMEPAD_X_MIN, GAMEPAD_X_MAX, -255, 255) : 0;
  int axisY = (abs(package.axisY) >= GAMEPAD_DEAD_ZONE_Y) ? map(package.axisY, GAMEPAD_Y_MIN, GAMEPAD_Y_MAX, -255, 255) : 0;
  int axisRX = (abs(package.axisRX) >= GAMEPAD_DEAD_ZONE_RX) ? map(package.axisRX, GAMEPAD_RX_MIN, GAMEPAD_RX_MAX, -255, 255) : 0;
  int axisRY = (abs(package.axisRY) >= GAMEPAD_DEAD_ZONE_RY) ? map(package.axisRY, GAMEPAD_RY_MIN, GAMEPAD_RY_MAX, -255, 255) : 0;
  int axisT = map((int16_t)package.throttle - (int16_t)package.brake, GAMEPAD_T_MIN, GAMEPAD_T_MAX, -255, 255);

  int vx = -axisY;
  int rz = -axisX;

  rz /= 1.5;
  if (!(package.buttons & GAMEPAD_BUTTON_M3))
  {
    vx /= 2;
    rz /= 2;
  }
  if ((package.buttons & GAMEPAD_BUTTON_M4))
  {
    vx /= 2;
    rz /= 2;
  }

  bot.chassis.setVelocities(vx, rz, true);



  if (package.buttons & GAMEPAD_BUTTON_R)
    commandInterpretator('x');
  else
  {
    int r_base = -axisRX/2;
    int r_arm = -axisRY/2;
    int r_claw = axisT;

    if (!(package.buttons & GAMEPAD_BUTTON_M3))
    {
      r_base /= 2;
      r_arm /= 2;
    }
    if ((package.buttons & GAMEPAD_BUTTON_M4))
    {
      r_base /= 2;
      r_arm /= 2;
    }

    bot.hand.setVelocities(r_base, r_arm, r_claw);
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
  }
}

void enableSensors(bool en)
{
  bot.enableSensors = en;

  display.items[LCD_TRACKING_SENSOR_LEFT]->enable(en);
  display.items[LCD_TRACKING_SENSOR_CENTER]->enable(en);
  display.items[LCD_TRACKING_SENSOR_RIGHT]->enable(en);
  display.items[LCD_BUMPER_SENSOR_LEFT]->enable(en);
  display.items[LCD_BUMPER_SENSOR_RIGHT]->enable(en);
  display.items[LCD_DISTANCE_SENSOR]->enable(en);
}

void enableServos(bool en)
{
  !en ? bot.hand.detach() : bot.hand.attach();

  display.items[LCD_A1]->enable(en);
  display.items[LCD_A2]->enable(en);
  display.items[LCD_A3]->enable(en);
}

void commandInterpretator(char cmd)
{
    static uint8_t counter = 0;
    counter++;
    display.items[LCD_COMMAND_COUNTER]->set(counter);
    display.items[LCD_COMMAND]->set(cmd);
    DebugWrite("Command", cmd);

    switch (cmd)
    {
      case 'F': startProgram(PRG_MOVING_FORWARD);       break;
      case 'B': startProgram(PRG_MOVING_BACKWARD);      break;
      case 'L': startProgram(PRG_TURNING_LEFT);         break;
      case 'R': startProgram(PRG_TURNING_RIGHT);        break;
      case 'G':
      case 'S': startProgram(PRG_NONE); bot.chassis.stop(); break;

      case 'X': speed = SPEED_LOW;      break;
      case 'Y': speed = SPEED_MEDIUM;   break;
      case 'Z': speed = SPEED_HIGH;     break;
      case 'A': startProgram(PRG_AVOIDANCE);      break;
      case 'D': startProgram(PRG_ANTIDROP);       break;
      case 'W': startProgram(PRG_FOLLOWING);      break;
      case 'T': startProgram(PRG_LINE_TRACKING);  break;

      case '1': enableSensors(!bot.enableSensors);  break;
      case '2': enableServos(!bot.hand.attached());  break;

      case 'o': startProgram(PRG_CLAW_OPENING);         break;
      case 'c': startProgram(PRG_CLAW_CLOSING);         break;
      case 'u': startProgram(PRG_ARM_DESCENDING);       break;
      case 'd': startProgram(PRG_ARM_RISING);           break;
      case 'l': startProgram(PRG_BASE_TURNING_LEFT);    break;
      case 'r': startProgram(PRG_BASE_TURNING_RIGHT);   break;
      case 'x': bot.hand.moveToDefault(); break;
      case 's': startProgram(PRG_NONE); bot.hand.stop(); break;

      case 'm':
        if (programMayBeRewrite)
          nActions = 0;
        programMayBeRewrite = false;
        storePosition();
        break;
      case 'a':
        programMayBeRewrite = true;
        currentAction = 0;
        if (nActions)
          startProgram(PRG_MEMORY_ACTION);
        break;

    }
}

void IR_control()
{
  static uint8_t old = IR_KEYCODE_OK;

  if (ir.available(true))
  {
    uint8_t code = ir.readCommand();
    if (old == code)
      return;
    old = code;

    char command = 0;
    switch (code)
    {
      //case IR_KEYCODE_1: command = ''; break;
      case IR_KEYCODE_2: command = 'u'; break;
      //case IR_KEYCODE_3: command = ''; break;
      case IR_KEYCODE_4: command = 'l'; break;
      case IR_KEYCODE_5: command = 'x'; break;
      case IR_KEYCODE_6: command = 'r'; break;
      case IR_KEYCODE_7: command = 'o'; break;
      case IR_KEYCODE_8: command = 'd'; break;
      case IR_KEYCODE_9: command = 'c'; break;
      //case IR_KEYCODE_0: command = ''; break;
      //case IR_KEYCODE_STAR: command = ''; break;
      //case IR_KEYCODE_POUND: command = ''; break;
      case IR_KEYCODE_UP: command = 'F'; break;
      case IR_KEYCODE_DOWN: command = 'B'; break;
      case IR_KEYCODE_OK: command = 's'; break;
      case IR_KEYCODE_LEFT: command = 'L'; break;
      case IR_KEYCODE_RIGHT: command = 'R'; break;
    };
    if (command)
      commandInterpretator(command);
  }
  else if (ir.timeout(200)) // ждём таймаут от последнего кода и стоп
  {
    commandInterpretator(bot.chassis.isMoving() ? 'S' : 's');
    old = IR_KEYCODE_OK;
  }
}

int packetSizeErrorCounter = 0;
int packetCrcErrorCounter = 0;

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
      ++packetSizeErrorCounter;
      DebugWrite("ERROR: size of data package=", res);
      lcdSizeError();
      return;
    }
    int8_t crc = -1;
    res = Serial.readBytes((char*)&crc, 1);
#if CHECK_GAMEPAD_CRC == 1
    int8_t crc2 = calcCRC8((byte*)&package, sizeof(package));
    if (crc != crc2)
    {
      ++packetCrcErrorCounter;
      DebugWrite("ERROR: crc of data package=", crc);
      lcdCrcError();
      return;
    }
#endif
    gamepadControl(package);
  }
  else
    commandInterpretator(ch);
}

void displayInit()
{
  display.items[LCD_FPS] = new LcdInt(0, 3, 6);
  display.items[LCD_TRACKING_SENSOR_LEFT] = new LcdBool(15, 1, '^', 'o');
  display.items[LCD_TRACKING_SENSOR_CENTER] = new LcdBool(16, 1, '^', 'o');
  display.items[LCD_TRACKING_SENSOR_RIGHT] = new LcdBool(17, 1, '^', 'o');
  display.items[LCD_BUMPER_SENSOR_LEFT] = new LcdBool(14, 2, '<', 'o');
  display.items[LCD_BUMPER_SENSOR_RIGHT] = new LcdBool(18, 2, '>', 'o');
  display.items[LCD_DISTANCE_SENSOR] = new LcdInt(15, 3, 3);
  display.items[LCD_COMMAND_COUNTER] = new LcdInt(0, 0, 3);
  display.items[LCD_COMMAND] = new LcdChar(5, 0);

  display.items[LCD_A1] = new LcdInt(7,  0, 4);
  display.items[LCD_A2] = new LcdInt(11, 0, 4);
  display.items[LCD_A3] = new LcdInt(15, 0, 4);
}

void displayUpdate()
{
  display.items[LCD_FPS]->set(fps);
  if (bot.enableSensors)
  {
    display.items[LCD_TRACKING_SENSOR_LEFT]->set(bot.trackingSensorLeft);
    display.items[LCD_TRACKING_SENSOR_CENTER]->set(bot.trackingSensorCenter);
    display.items[LCD_TRACKING_SENSOR_RIGHT]->set(bot.trackingSensorRight);
    display.items[LCD_BUMPER_SENSOR_LEFT]->set(bot.bumperSensorLeft);
    display.items[LCD_BUMPER_SENSOR_RIGHT]->set(bot.bumperSensorRight);
    display.items[LCD_DISTANCE_SENSOR]->set(bot.distanceSensor);
  }
  display.items[LCD_A1]->set(MKS2DEG(DESCALE(bot.hand.current_pos[0])));
  display.items[LCD_A2]->set(MKS2DEG(DESCALE(bot.hand.current_pos[1])));
  display.items[LCD_A3]->set(MKS2DEG(DESCALE(bot.hand.current_pos[2])));

  display.update();
}

void irIsr() {
    ir.tick();
}

void setup()
{
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(SERIAL_TIMEOUT);
  Serial.println("Start");

  lcd.init();
  lcd.backlight();

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

  attachInterrupt(digitalPinToInterrupt(PIN_IR), irIsr, FALLING);

  displayInit();
}


void loop()
{
  calcFps();

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

  displayUpdate();

  bot.tick();
}
