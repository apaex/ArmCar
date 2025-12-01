#include <CRC.h>
#define NEC_SKIP_REPEATS 0
#include <NecDecoder.h>
#include <Beeper.h>
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
Beeper buz(PIN_BEEPER);

NecDecoder ir;
Bot bot;

int chassis_speed = SPEED_MEDIUM;

enum Program
{
  PRG_NONE,
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
  LCD_MEMORY,
  LCD_SELECTABLE_PROGRAM,
  _LCD_COUNT,
};

LcdItemBase* lcdItems[_LCD_COUNT];
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
  if (bot.trackingSensorLeft && !bot.trackingSensorCenter && bot.trackingSensorRight)             // ░ ▓ ░
    bot.chassis.moveForward(SPEED_LINE_TRACKING);

  else if (!bot.trackingSensorLeft && bot.trackingSensorRight)                                    // ▓ * ░
    bot.chassis.rotateLeft(bot.trackingSensorCenter ? SPEED_LINE_TRACKING :  SPEED_LINE_TRACKING*2/3);

  else if (bot.trackingSensorLeft && !bot.trackingSensorRight)                                    // ░ * ▓
    bot.chassis.rotateRight(bot.trackingSensorCenter ? SPEED_LINE_TRACKING : SPEED_LINE_TRACKING*2/3);

  else if (!bot.trackingSensorLeft && !bot.trackingSensorCenter && !bot.trackingSensorRight)      // ▓ ▓ ▓
    bot.chassis.stop();
}

void delay_(unsigned long ms)
{
  unsigned long start = millis();
  while (millis() - start < ms)
  {
    calcFps();

    IR_control();
    UART_control();

    bot.readSensors();
    displayUpdate();
    bot.tick();
  }
}

void Anti_drop_Function() //не падать со стола
{
  if (bot.trackingSensorLeft && bot.trackingSensorCenter && bot.trackingSensorRight)
    bot.chassis.moveForward(60);
  else
  {
    bot.chassis.moveBackward(60);
    delay_(600);
    bot.chassis.rotateLeft(60);
    delay_(500);
  }
}

void Following_Function() //преследование
{
  if (bot.distanceSensor < 15)
    bot.chassis.moveBackward(SPEED_FOLLOWING * 0.8);
  else if (bot.distanceSensor <= 20)
    bot.chassis.stop();
  else if (bot.distanceSensor <= 25)
    bot.chassis.moveForward(SPEED_FOLLOWING * 0.8);
  else if (bot.distanceSensor <= 30)
    bot.chassis.moveForward(SPEED_FOLLOWING);
  else
    bot.chassis.stop();
}

void Avoidance_Function() //объезд препятствий
{
  if (bot.distanceSensor <= 7)  // отъезжаем
  {
    //bot.chassis.stop(); ???
    //delay_(100);
    bot.chassis.moveBackward(SPEED_AVOIDANCE * 0.7);
    //delay_(60); ???
  }
  else if (bot.distanceSensor <= 15)  // отворачиваем
  {
    //bot.chassis.stop(); ???
    //delay_(100);

    byte rot = 0;
    if (!bot.bumperSensorLeft && !bot.bumperSensorRight)
      rot = random(2) ? -1 : 1;
    else if (!bot.bumperSensorLeft)
      rot = -1;
    else if (!bot.bumperSensorRight)
      rot = 1;

    switch (rot)
    {
    case -1: bot.chassis.rotateLeft(SPEED_AVOIDANCE * 0.7); break;
    case  1: bot.chassis.rotateRight(SPEED_AVOIDANCE * 0.7); break;
    case  0: bot.chassis.moveBackward(SPEED_AVOIDANCE * 0.7); break;
    }

    delay_(600);
  }
  else
    bot.chassis.moveForward(SPEED_AVOIDANCE);
}

void auto_do()
{
  if (!bot.hand.isReady())
    return;

  if (++currentAction >= nActions)
    currentAction = 0;

  bot.hand.moveTo(mem[currentAction]);
}

static Program selectable_program = PRG_NONE;

void selectProgram()
{
  if (selectable_program >= PRG_AVOIDANCE && selectable_program < PRG_LINE_TRACKING)
    selectable_program = Program((int)selectable_program + 1);
  else
    selectable_program = PRG_AVOIDANCE;

  static const char* map[] = {
    "AVOIDANCE",
    "FOLLOWING",
    "ANTIDROP",
    "LINE_TRACKING",
  };

  display.items[LCD_SELECTABLE_PROGRAM]->enable(true);
  display.items[LCD_SELECTABLE_PROGRAM]->set(map[selectable_program - PRG_AVOIDANCE]);

  commandInterpretator('S');
}

void startProgram()
{
  display.items[LCD_SELECTABLE_PROGRAM]->enable(false);
  if (program == selectable_program)
    commandInterpretator('S');
  else
  {
    //startProgram(selectable_program); пока через команды
    switch (selectable_program)
    {
    case PRG_AVOIDANCE: commandInterpretator('A'); break;
    case PRG_FOLLOWING: commandInterpretator('W'); break;
    case PRG_ANTIDROP: commandInterpretator('D'); break;
    case PRG_LINE_TRACKING: commandInterpretator('T'); break;
    }
  }
}

void setProgram(Program _program)
{
  program = _program;
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
      case 'F': setProgram(PRG_NONE); bot.chassis.moveForward(chassis_speed);   break;
      case 'B': setProgram(PRG_NONE); bot.chassis.moveBackward(chassis_speed);  break;
      case 'L': setProgram(PRG_NONE); bot.chassis.rotateLeft(chassis_speed);    break;
      case 'R': setProgram(PRG_NONE); bot.chassis.rotateRight(chassis_speed);   break;
      case 'G':
      case 'S': setProgram(PRG_NONE); bot.chassis.stop();                       break;

      case 'X': chassis_speed = SPEED_LOW;      break;
      case 'Y': chassis_speed = SPEED_MEDIUM;   break;
      case 'Z': chassis_speed = SPEED_HIGH;     break;
      case 'A': setProgram(PRG_AVOIDANCE);      break;
      case 'D': setProgram(PRG_ANTIDROP);       break;
      case 'W': setProgram(PRG_FOLLOWING);      break;
      case 'T': setProgram(PRG_LINE_TRACKING);  break;

      case '1': enableSensors(!bot.enableSensors);  break;
      case '2': enableServos(!bot.hand.attached());  break;
      case 'x': setProgram(PRG_NONE); bot.hand.moveToDefault(); break;

      case 'o': setProgram(PRG_NONE); bot.hand.clawOpen(120);     break;
      case 'c': setProgram(PRG_NONE); bot.hand.clawClose(120);    break;
      case 'u': setProgram(PRG_NONE); bot.hand.armDescend(60);    break;
      case 'd': setProgram(PRG_NONE); bot.hand.armRise(60);       break;
      case 'l': setProgram(PRG_NONE); bot.hand.baseTurnLeft(80);  break;
      case 'r': setProgram(PRG_NONE); bot.hand.baseTurnRight(80); break;
      case 's': setProgram(PRG_NONE); bot.hand.stop();            break;

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
          setProgram(PRG_MEMORY_ACTION);
        break;
    }
}

void gamepadControl(const GamepadData &package)
{
  static uint16_t buttons = 0;
  static uint8_t miscButtons = 0;
  static uint8_t dpad = 0;

  int axisX = (abs(package.axisX) >= GAMEPAD_DEAD_ZONE_X) ? map(package.axisX, GAMEPAD_X_MIN, GAMEPAD_X_MAX, -255, 255) : 0;
  int axisY = (abs(package.axisY) >= GAMEPAD_DEAD_ZONE_Y) ? map(package.axisY, GAMEPAD_Y_MIN, GAMEPAD_Y_MAX, -255, 255) : 0;
  int axisRX = (abs(package.axisRX) >= GAMEPAD_DEAD_ZONE_RX) ? map(package.axisRX, GAMEPAD_RX_MIN, GAMEPAD_RX_MAX, -255, 255) : 0;
  int axisRY = (abs(package.axisRY) >= GAMEPAD_DEAD_ZONE_RY) ? map(package.axisRY, GAMEPAD_RY_MIN, GAMEPAD_RY_MAX, -255, 255) : 0;
  int axisT = map((int16_t)package.throttle - (int16_t)package.brake, GAMEPAD_T_MIN, GAMEPAD_T_MAX, -255, 255);

  int vx = -axisY;
  int rz = -axisX;

  vx /= 2;
  rz /= 4;

  if ((package.buttons & GAMEPAD_BUTTON_M4))
  {
    vx *= 2;
    rz *= 2;
  }

  bot.chassis.setVelocities(vx, rz);


  if (package.buttons & GAMEPAD_BUTTON_R)
    commandInterpretator('x');
  else if (program != PRG_MEMORY_ACTION)
  {
    int r_base = -axisRX;
    int r_arm = -axisRY;
    int r_claw = axisT;

    r_base /= 1.5;
    r_arm /= 2;

    if ((package.buttons & GAMEPAD_BUTTON_M4))
    {
      r_base *= 2;
      r_arm *= 2;
    }

    bot.hand.setVelocities(r_base, r_arm, r_claw);
  }


  if (~buttons & package.buttons & GAMEPAD_BUTTON_A)
    commandInterpretator('m');
  else if (~buttons & package.buttons & GAMEPAD_BUTTON_B)
    commandInterpretator((program == PRG_MEMORY_ACTION) ? 's' : 'a');

  if (~miscButtons & package.miscButtons & GAMEPAD_BUTTON_MISC_SELECT)
    selectProgram();
  else if (~miscButtons & package.miscButtons & GAMEPAD_BUTTON_MISC_START)
    startProgram();

  buttons = package.buttons;
  miscButtons = package.miscButtons;
  dpad = package.dpad;
}

void IR_control()
{
  static uint8_t old = 0;
  static char stop = 0;

  if (ir.available(true))
  {
    uint8_t code = ir.readCommand();
    if (old == code)
      return;
    old = code;
    stop = 0;

    switch (code)
    {
      case IR_KEYCODE_UP:     commandInterpretator('F'); stop = 'S'; break;
      case IR_KEYCODE_DOWN:   commandInterpretator('B'); stop = 'S'; break;
      case IR_KEYCODE_LEFT:   commandInterpretator('L'); stop = 'S'; break;
      case IR_KEYCODE_RIGHT:  commandInterpretator('R'); stop = 'S'; break;

      case IR_KEYCODE_4:      commandInterpretator('l'); stop = 's'; break;
      case IR_KEYCODE_6:      commandInterpretator('r'); stop = 's'; break;
      case IR_KEYCODE_2:      commandInterpretator('u'); stop = 's'; break;
      case IR_KEYCODE_8:      commandInterpretator('d'); stop = 's'; break;
      case IR_KEYCODE_7:      commandInterpretator('c'); stop = 's'; break;
      case IR_KEYCODE_9:      commandInterpretator('o'); stop = 's'; break;
      case IR_KEYCODE_OK:     commandInterpretator('s'); break;
      case IR_KEYCODE_5:      commandInterpretator('x'); break;

      case IR_KEYCODE_STAR:   selectProgram(); break;
      case IR_KEYCODE_POUND:  startProgram(); break;
      case IR_KEYCODE_1:      commandInterpretator('m'); break;
      case IR_KEYCODE_3:      commandInterpretator((program == PRG_MEMORY_ACTION) ? 's' : 'a'); break;
      case IR_KEYCODE_0:      break;
    };
  }
  else if (ir.timeout(100)) // ждём таймаут от последнего кода и стоп
  {
    if (stop)
      commandInterpretator(stop);
    old = 0;
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
  display.items[LCD_FPS] = new LcdItem<uint32_t>((byte)0, (byte)3, (byte)6, "%6lu");
  display.items[LCD_TRACKING_SENSOR_LEFT] = new LcdItem<bool>(16, 2, 1, ".-");
  display.items[LCD_TRACKING_SENSOR_CENTER] = new LcdItem<bool>(17, 2, 1, ".-");
  display.items[LCD_TRACKING_SENSOR_RIGHT] = new LcdItem<bool>(18, 2, 1, ".-");
  display.items[LCD_BUMPER_SENSOR_LEFT] = new LcdItem<bool>(15, 2, 1, "o[");
  display.items[LCD_BUMPER_SENSOR_RIGHT] = new LcdItem<bool>(19, 2, 1, "o]");
  display.items[LCD_DISTANCE_SENSOR] = new LcdItem<uint8_t>(16, 3, 3, "%3u");
  display.items[LCD_COMMAND_COUNTER] = new LcdItem<uint8_t>(0, 0, 3, "%3u");
  display.items[LCD_COMMAND] = new LcdItem<char>(4, 0, 1, "%c");

  display.items[LCD_A1] = new LcdItem<int16_t>(8,  0, 4, "%4d");
  display.items[LCD_A2] = new LcdItem<int16_t>(12, 0, 4, "%4d");
  display.items[LCD_A3] = new LcdItem<int16_t>(16, 0, 4, "%4d");

  display.items[LCD_MEMORY] = new LcdItem<const char *>(7, 3, 4, "M:%-2u");
  display.items[LCD_SELECTABLE_PROGRAM] = new LcdItem<const char *>(0, 1, 13, "%-13s");
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
  if (bot.hand.attached())
  {
    display.items[LCD_A1]->set(MKS2DEG(DESCALE(bot.hand.current_pos[0])));
    display.items[LCD_A2]->set(MKS2DEG(DESCALE(bot.hand.current_pos[1])));
    display.items[LCD_A3]->set(MKS2DEG(DESCALE(bot.hand.current_pos[2])));
  }
  display.items[LCD_MEMORY]->set(nActions);

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

  int skill[4][N_SERVOS] = {
    {SCALE(90), SCALE(30), SCALE(120)},
    {SCALE(90), SCALE(120), SCALE(120)},
    {SCALE(90), SCALE(120), SCALE(170)},
    {SCALE(90), SCALE(50), SCALE(170)}
  };

  for (int i=0; i<4; ++i)
    mem[nActions++] = HandPosition(skill[i]);

  buz.useTone(true);
  tone(PIN_BEEPER, 1000, 100);
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

  if (program == PRG_NONE && bot.chassis.getState() == MOVING_FORWARD)  // ручное движение
  {
    if (bot.bumperSensorLeft || bot.bumperSensorRight || bot.distanceSensor < 5)  // проверка бамперов
    {
      buz.beep(1000, 3, 100, 100);
      bot.chassis.stop();
    }
  }

  displayUpdate();

  buz.tick();
  bot.tick();
}
