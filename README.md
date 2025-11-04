# LAFVIN Smart Robot Arm Car
Альтернативная прошивка для робота [LAFVIN Mechanical 4WD Robot Arm Car Kit for Arduino Robot Car Robot Arm Programmable STEM Toys/Support Android](https://lafvintech.com/products/lafvin-mechanical-4wd-robot-arm-car-kit-for-arduino-robot-car-robot-arm-programmable-stem-toys-support-android-1)

Все функции оригинальной прошивки сохранены, а также есть полная совместимость с оригинальным железом.

* Прошивка полностью написана заново, имеет намного более качественное управление роботом
* Поддерживаются 2 ИК-дальномера бамперов
* Поддерживается ЖК-дисплей
* Поддерживается геймпад

Сборка прошивки, в отличие от оригинальной, требует установки некоторого количества библиотек. 
* Servo by Michael Margolis, Arduino Version 1.1.2 
* LiquidCrystal I2C by Frank de Brabander Version 1.1.2 
* CRC by Rob Tillaart <rob.tillaart@gmail.com> Version 1.0.3 
* NecDecoder by AlexGyver <alex@alexgyver.ru> Version 3.0.2 
* GyverMotor by AlexGyver <alex@alexgyver.ru> Version 4.1.1 
* GyverIO by AlexGyver <alex@alexgyver.ru> Version 1.3.11 
* GyverPing by AlexGyver <alex@alexgyver.ru> Version 1.0.1 

Обратите внимание на версии библиотек. Если программа не собирается или не работает с другой версией - установите указанные, на которых тестировалась работа.
  
TODO:
* Пока не работают программы антидроп, преследование и убегание
