/*
  Скетч к проекту "Часы на ГРИ версия 2"
*/

/*
	- Отладка - установить #define DEBUG 1
      - При включении питания все индикаторы будут синхронно пробегать от 0 до 9
      - При включении питания происходит безусловный сброс RTC (RTC заменит свое время на время из компа)
      - Информация из RTC DS3231 выводится в порт монитора  а) при включении питания б) по завершении п.6 настройки в) каждые 10 мин хода часов
      - Порт монитора отображает значения с датчика температуры, когда #define TEMP 1

  -  В РЕЖИМЕ ЧАСОВ каждую минуту часы показывают:
    с 40 по 45sec - день/месяц(Россия) или месяц/день(USA),  с 46 по 50sec-температура(-tt-)(*), остальнoе - часы/минуты
      (*) - If TEMP 1 or 2 is selected and DS18B20 sensor is soldered to Arduino: "5V" to pin27 (5v), "GND" to pin29 (GND), "OUT" to (D2),
	        solder 4.7k resistor between 5v and OUT pins

  - Настройки - Левая кнопка - "выбор", остальные "больше" и "меньше"
    Последовательность Настройки:
    1. Первый Клик по "выбору" - отображаются месяц/день, мигает месяц (левые 2 индикатора - USA, или правые 2 индикатора - Россия),
       - Клик "больше" и "меньше" - изменение месяца 1-12
    2. Следующий Клик по "выбору" - отображаются месяц/день, мигает день месяца (правые 2 индикатора - USA, или левые 2 индикатора - Россия),
       - Клик "больше" и "меньше" - изменение дня месяца 1-31
    3. Следующий Клик по "выбору" - отображается день недели(dd.dd), мигает день недели,
       - Клик "больше" и "меньше" - изменение дня недели 1-7
    4. Следующий Клик по "выбору" - отображается температура,
       - Клик "больше" и "меньше" - изменение температуры на один градус на каждое нажатие кнопки
    5. Следующий Клик по "выбору" - отображаются часы/минуты, мигают часы (левые 2 индикатора),
       - Клик "больше" и "меньше" - изменение часов 00-23
    6. Следующий Клик по "выбору" - отображаются часы/минуты, мигают минуты (правые 2 индикатора),
       - Клик "больше" и "меньше" - изменение минут 00-59
    7. Следующий Клик по "выбору" - выход из настройки, данные сохранятся в RTC DS3231, возврат в РЕЖИМ ЧАСОВ - отображаются часы/минуты
    Прим.  Точка не светит при настройке

  - Управление эффектами В РЕЖИМЕ ЧАСОВ:

    - Удержание центральной кнопки включает и выключает "глюки"
    - Клик по центральной кнопке переключает режимы подсветки ламп
      - Дыхание
      - Постоянное свечение
      - Отключена

	- Удержание правой кнопки включает loop эффект - каждую минуту новый эффект перелистывания цифр
    - Клик по правой кнопке выключает loop и переключает режимы перелистывания цифр
      - Без эффекта
      - Плавное угасание
      - Перемотка по порядку числа
      - Перемотка по катодам
      - Поезд
      - Резинка
*/

// ************************** НАСТРОЙКИ **************************

// Тип платы часов:
// 0 - IN-12 (Индикаторы стоят правильно)
// 1 - IN-12 (Индикаторы перевёрнуты)
// 2 - IN-14 (Обычная и neon dot)
// 3 - другие индикаторы
#define BOARD_TYPE 2

#define DUTY 350  // Шим теперь 9 битный (512 значений)

#define DEBUG 0      // 0 - debug is off, 1 - для проверки индикаторов, RTC и показа данных температуры  
#define COUNTRY 1    // Формат даты: 0 - нет, 1 - Россия (ДД.ММ), 2 - США (ММ.ДД)
#define TEMP 1       // Температура: 0 - выкл, 1 - Цельсии, 2 - Фаренгейты
#define CLOCK_IND 0  // Формат времени: 0 - 24 часа, 1 - 12 часов

// ======================= ЭФФЕКТЫ =======================
byte FLIP_EFFECT = 1;  // Эффекты переключения цифр:
// Выбранный эффект активен при первом запуске и меняется кнопками. Запоминается в память
// 0 - Нет эффекта
// 1 - Плавное угасание и появление (рекомендуемая скорость: 50-100)
// 2 - Перемотка по порядку числа (рекомендуемая скорость: 50-80)
// 3 - Перемотка по порядку катодов в лампе (рекомендуемая скорость: 30-50)
// 4 - Поезд (рекомендуемая скорость: 50-170)
// 5 - Резинка (рекомендуемая скорость: 50-150)

// =======================  ЯРКОСТЬ =======================
#define NIGHT_LIGHT 1       // Менять яркость от времени суток (0 - выкл, 1 - вкл)
#define NIGHT_START 0       // Час перехода на ночную подсветку
#define NIGHT_END 5         // Час перехода на дневную подсветку

#define INDI_BRIGHT 23      // Яркость цифр дневная (1 - 24) !на 24 могут быть фантомные цифры!
#define INDI_BRIGHT_N 10    // Яркость цифр ночная (1 - 24)

#define DOT_BRIGHT 80       // Яркость точки дневная (1 - 255)
#define DOT_BRIGHT_N 30     // Яркость точки ночная (1 - 255)

#define BACKL_BRIGHT 250    // Яркость подсветки ламп дневная (0 - 255, 0 - подсветка выключена)
#define BACKL_BRIGHT_N 30   // Яркость подсветки ламп ночная (0 - 255, 0 - подсветка выключена)

#define BACKL_MIN_BRIGHT 10 // Мин. яркость подсветки ламп в режиме дыхание (0 - 255)
#define BACKL_PAUSE 400     // Пауза "темноты" между вспышками подсветки ламп в режиме дыхание, мс
#define BACKL_STEP 2        // Шаг мигания подсветки
#define BACKL_TIME 5000     // Период подсветки, мс

// =======================  ГЛЮКИ =======================
#define GLITCH_MIN 30       // Минимальное время между глюками, с
#define GLITCH_MAX 120      // Максимальное время между глюками, с

// ======================  МИГАНИЕ =======================
#define DOT_TIME 500        // Время мигания точки, мс
#define DOT_TIMER 50        // Шаг яркости точки, мс

// ==================  АНТИОТРАВЛЕНИЕ ====================
#define BURN_TIME 10        // Период обхода индикаторов в режиме очистки, мс
#define BURN_LOOPS 3        // Количество циклов очистки за каждый период
#define BURN_PERIOD 20      // Период антиотравления, минут


// *********************** ДЛЯ РАЗРАБОТЧИКОВ ***********************
byte BACKL_MODE = 1;                              // Выбранный режим подсветки активен при запуске и меняется кнопками
byte FLIP_SPEED[] = {0, 70, 50, 40, 70, 70};      // Скорость эффектов переключения индикаторов, мс
byte FLIP_EFFECT_NUM = sizeof(FLIP_SPEED);        // Количество эффектов переключения индикаторов
bool GLITCH_ALLOWED = 0;                          // 1 - Включить, 0 - Выключить глюки. Управляются удерживанием центральной кнопки
bool EFFECT_LOOP = 0;                             // Эффекты 1 - в цикле, 0 - из памяти

// Пины
#define KEY0 3    // часы
#define KEY1 4    // часы
#define KEY2 5    // минуты
#define KEY3 6    // минуты
#define BTN1 7    // кнопка 1
#define BTN2 8    // кнопка 2
#define GEN  9    // генератор
#define DOT 10    // точка
#define BACKL 11  // подсветка
#define BTN3  12  // кнопка 3

// Режимы настройки
#define CLOCK 0    // 0-clock 
#define MONTH 1    // 1-set month
#define DAY   2    // 2-set day of month
#define DAYW  3    // 3-set day of week
#define SET_TEMP 4 // 4-set temperature
#define HRS   5    // 5-set hrs
#define MIN   6    // 6-set mins

#define STOPWATCH_READY 7    // Секундомер готов к запуску
#define STOPWATCH_RUN 8      // Секундомер работает
#define STOPWATCH_STOP 9     // Секундомер остановлен

// Переменные для секундомера
unsigned long stopwatchStart;
unsigned long stopwatchPrev;
unsigned long stopwatchCur;
bool stopwatchRun = false;
bool stopwatchFinish = false;

/*
  ард ног ном
  А0  7   4
  А1  6   2
  А2  4   8
  А3  3   1
*/

// Дешифратор
#define DECODER0 A0
#define DECODER1 A1
#define DECODER2 A2
#define DECODER3 A3

// Распиновка ламп
#if (BOARD_TYPE == 0)   // 0 - IN-12 (индикаторы стоят правильно)
const byte digitMask[] = {7, 3, 6, 4, 1, 9, 8, 0, 5, 2};   // маска дешифратора платы in12_turned (цифры нормальные)
const byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // порядок индикаторов слева направо
const byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // порядок катодов in12

#elif (BOARD_TYPE == 1)  // 1 - IN-12 (индикаторы перевёрнуты)
const byte digitMask[] = {2, 8, 1, 9, 6, 4, 3, 5, 0, 7};   // маска дешифратора платы in12 (цифры вверх ногами)
const byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // порядок индикаторов справа налево (для IN-12 turned) и ин-14
const byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // порядок катодов in12

#elif (BOARD_TYPE == 2) // 2 - IN-14 (обычная и neon dot) 
const byte digitMask[] = {9, 8, 0, 5, 4, 7, 3, 6, 2, 1};   // маска дешифратора платы in14
const byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // порядок индикаторов справа налево (для IN-12 turned) и ин-14
const byte cathodeMask[] = {1, 0, 2, 9, 3, 8, 4, 7, 5, 6}; // порядок катодов in14

#elif (BOARD_TYPE == 3)  // 3 - другие индикаторы
const byte digitMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};   // тут вводим свой порядок пинов
const byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // свой порядок индикаторов
const byte cathodeMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; // и свой порядок катодов

#endif

// Библиотеки
#include "timer2Minim.h"
#include <GyverButton.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

#include "microDS18B20.h"
MicroDS18B20<2> sensor;

RTC_DS3231 rtc;

timerMinim dotTimer(500);                // Полсекундный таймер для часов
timerMinim dotBrightTimer(DOT_TIMER);    // Таймер шага яркости точки
timerMinim backlBrightTimer(30);         // Таймер шага яркости подсветки
timerMinim flipTimer(FLIP_SPEED[FLIP_EFFECT]);
timerMinim glitchTimer(1000);
timerMinim blinkTimer(500);

// Кнопки
GButton btnSet(BTN1, HIGH_PULL, NORM_OPEN);
GButton btnL(BTN2, HIGH_PULL, NORM_OPEN);
GButton btnR(BTN3, HIGH_PULL, NORM_OPEN);

// Переменные
volatile int8_t indiDimm[4];      // Величина диммирования (0-24)
volatile int8_t indiCounter[4];   // Счётчик каждого индикатора (0-24)
volatile int8_t indiDigits[4];    // Цифры, которые должны показать индикаторы (0-10)
volatile int8_t curIndi;          // Текущий индикатор (0-3)

bool dotFlag;
int8_t hrs, mins, secs;
int8_t DayWeek, Month, Day;
int Year;
int8_t changeHrs = 14, changeMins=27, changeDayWeek, changeMonth, changeDay;
const byte DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
float meas;
float temp_delta = 0;

byte indiMaxBright = INDI_BRIGHT;
bool indiBrightDirection, indiState;
int indiBrightCounter;

byte dotMaxBright = DOT_BRIGHT, dotBrightStep;
bool dotBrightFlag, dotBrightDirection;
int dotBrightCounter;

byte backlMaxBright = BACKL_BRIGHT;
bool backlBrightFlag, backlBrightDirection;
int backlBrightCounter;

byte glitchCounter, glitchMax, glitchIndic;
bool glitchFlag;

byte curMode = 0;  // Настройки: 0-indication 1-set month 2-set day of month 3-set day of week 4-set temperature 5-set hrs 6-set mins
byte curStatus = 0;  // Статус индикации: 0-hrs/min 1-month/day 2-day of week 3- temperature

bool anodeStates[] = {1, 1, 1, 1};
bool flipIndics[4];
bool newTimeFlag = false;
bool flipInit = false;
bool lampState = false;
bool trainLeaving = false;

byte newTime[4];
byte startCathode[4], endCathode[4];
byte currentLamp, flipEffectStages;

const uint8_t CRTgamma[256] PROGMEM = {
  0,    0,    1,    1,    1,    1,    1,    1,
  1,    1,    1,    1,    1,    1,    1,    1,
  2,    2,    2,    2,    2,    2,    2,    2,
  3,    3,    3,    3,    3,    3,    4,    4,
  4,    4,    4,    5,    5,    5,    5,    6,
  6,    6,    7,    7,    7,    8,    8,    8,
  9,    9,    9,    10,   10,   10,   11,   11,
  12,   12,   12,   13,   13,   14,   14,   15,
  15,   16,   16,   17,   17,   18,   18,   19,
  19,   20,   20,   21,   22,   22,   23,   23,
  24,   25,   25,   26,   26,   27,   28,   28,
  29,   30,   30,   31,   32,   33,   33,   34,
  35,   35,   36,   37,   38,   39,   39,   40,
  41,   42,   43,   43,   44,   45,   46,   47,
  48,   49,   49,   50,   51,   52,   53,   54,
  55,   56,   57,   58,   59,   60,   61,   62,
  63,   64,   65,   66,   67,   68,   69,   70,
  71,   72,   73,   74,   75,   76,   77,   79,
  80,   81,   82,   83,   84,   85,   87,   88,
  89,   90,   91,   93,   94,   95,   96,   98,
  99,   100,  101,  103,  104,  105,  107,  108,
  109,  110,  112,  113,  115,  116,  117,  119,
  120,  121,  123,  124,  126,  127,  129,  130,
  131,  133,  134,  136,  137,  139,  140,  142,
  143,  145,  146,  148,  149,  151,  153,  154,
  156,  157,  159,  161,  162,  164,  165,  167,
  169,  170,  172,  174,  175,  177,  179,  180,
  182,  184,  186,  187,  189,  191,  193,  194,
  196,  198,  200,  202,  203,  205,  207,  209,
  211,  213,  214,  216,  218,  220,  222,  224,
  226,  228,  230,  232,  233,  235,  237,  239,
  241,  243,  245,  247,  249,  251,  253,  255,
};

byte getPWM_CRT(byte val) {
  return pgm_read_byte(&(CRTgamma[val]));
}

// Быстрый digitalWrite
void setPin(uint8_t pin, uint8_t x) {
  switch (pin) {
    // откл pwm
    case 3:  // 2B
      bitClear(TCCR2A, COM2B1);
      break;
    case 5: // 0B
      bitClear(TCCR0A, COM0B1);
      break;
    case 6: // 0A
      bitClear(TCCR0A, COM0A1);
      break;
    case 9: // 1A
      bitClear(TCCR1A, COM1A1);
      break;
    case 10: // 1B
      bitClear(TCCR1A, COM1B1);
      break;
    case 11: // 2A
      bitClear(TCCR2A, COM2A1);
      break;
  }

  if (pin < 8) bitWrite(PORTD, pin, x);
  else if (pin < 14) bitWrite(PORTB, (pin - 8), x);
  else if (pin < 20) bitWrite(PORTC, (pin - 14), x);
  else return;
}

// Быстрый analogWrite
void setPWM(uint8_t pin, uint16_t duty) {
  if (duty == 0) setPin(pin, LOW);
  else {
    switch (pin) {
      case 5:
        bitSet(TCCR0A, COM0B1);
        OCR0B = duty;
        break;
      case 6:
        bitSet(TCCR0A, COM0A1);
        OCR0A = duty;
        break;
      case 10:
        bitSet(TCCR1A, COM1B1);
        OCR1B = duty;
        break;
      case 9:
        bitSet(TCCR1A, COM1A1);
        OCR1A = duty;
        break;
      case 3:
        bitSet(TCCR2A, COM2B1);
        OCR2B = duty;
        break;
      case 11:
        bitSet(TCCR2A, COM2A1);
        OCR2A = duty;
        break;
      default:
        break;
    }
  }
}

void setup() {
  if (DEBUG) Serial.begin(9600);

  randomSeed(analogRead(7));  // Случайное зерно для генератора случайных чисел

  // Настройка пинов на выход
  pinMode(DECODER0, OUTPUT);
  pinMode(DECODER1, OUTPUT);
  pinMode(DECODER2, OUTPUT);
  pinMode(DECODER3, OUTPUT);
  pinMode(KEY0, OUTPUT);
  pinMode(KEY1, OUTPUT);
  pinMode(KEY2, OUTPUT);
  pinMode(KEY3, OUTPUT);
  pinMode(GEN, OUTPUT);
  pinMode(DOT, OUTPUT);
  pinMode(BACKL, OUTPUT);

  // Частота ШИМ на 9 и 10 выводах 31 кГц
  TCCR1A = TCCR1A & 0xe0 | 2;
  TCCR1B = TCCR1B & 0xe0 | 0x09;

  // Включить ШИМ
  setPWM(GEN, DUTY);

  // Перенастроить частоту ШИМ на пинах 3 и 11 на 7.8 кГц и разрешить прерывания COMPA
  TCCR2B = (TCCR2B & B11111000) | 2;    // Делитель 8
  TCCR2A |= (1 << WGM21);   // Включить CTC режим для COMPA
  TIMSK2 |= (1 << OCIE2A);  // Включить прерывания по совпадению COMPA

  // ---------- RTC -----------
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Сброс RTC при потере питания
  }

  // EEPROM
  if (EEPROM.read(1023) != 100) {  // Первый запуск
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Безусловный сброс RTC при первом запуске
    EEPROM.put(1023, 100);
    EEPROM.put(0, EFFECT_LOOP);
    EEPROM.put(1, BACKL_MODE);
    EEPROM.put(2, GLITCH_ALLOWED);
    DayWeek = 7;
    EEPROM.put(3, DayWeek);
  }
  
  EEPROM.get(0, EFFECT_LOOP);
  EEPROM.get(1, BACKL_MODE);
  EEPROM.get(2, GLITCH_ALLOWED);
  EEPROM.get(3, DayWeek);

  // Считывание данных из  RTC и обновление показаний часов
  DateTime now = rtc.now();
  Year = now.year();
  Month = now.month();
  Day = now.day();
  hrs = now.hour();
  mins = now.minute();
  secs = now.second();
  showRTC();  //  Вывод данных RTC в порт монитора при отладке

  changeBright();  // Изменить яркость согласно времени суток
  
  // Стартовый период глюков
  glitchTimer.setInterval(random(GLITCH_MIN * 1000L, GLITCH_MAX * 1000L));

  // Скорость режима смены цифры на индикаторе
  flipTimer.setInterval(FLIP_SPEED[FLIP_EFFECT]);
}

void loop() {
  if (dotTimer.isReady() && curMode == CLOCK) calculateTime();  // Каждые 500 мс пересчёт и отправка времени
  if (newTimeFlag && curMode == CLOCK) flipTick();   // Перелистывание цифр
  dotBrightTick();                                // Управление точкой
  backlBrightTick();                              // Управление подсветкой ламп
  if (GLITCH_ALLOWED) glitchTick();               // Глюки
  buttonsTick();                                  // Кнопки
  settingsTick();                                 // Настройки
}

void backlBrightTick() {  
  if (curStatus == 0) {
    if (BACKL_MODE == 0 && backlBrightTimer.isReady()) {
      if (backlMaxBright > 0) {
        if (backlBrightDirection) {
          if (!backlBrightFlag) {
            backlBrightFlag = true;
            backlBrightTimer.setInterval((float)BACKL_STEP / backlMaxBright / 2 * BACKL_TIME);
          }
          backlBrightCounter += BACKL_STEP;
          if (backlBrightCounter >= backlMaxBright) {
            backlBrightDirection = false;
            backlBrightCounter = backlMaxBright;
          }
        }
        else {
          backlBrightCounter -= BACKL_STEP;
          if (backlBrightCounter <= BACKL_MIN_BRIGHT) {
            backlBrightDirection = true;
            backlBrightCounter = BACKL_MIN_BRIGHT;
            backlBrightTimer.setInterval(BACKL_PAUSE);
            backlBrightFlag = false;
          }
        }
        setPWM(BACKL, getPWM_CRT(backlBrightCounter));
      }
    }
    else if (BACKL_MODE == 1) setPWM(BACKL, backlMaxBright);
    else if (BACKL_MODE == 2) digitalWrite(BACKL, 0);
  }
  else if (curStatus == 1) setPWM(BACKL, backlMaxBright/3+1);  // Индикация день-месяц
  else if (curStatus == 2) digitalWrite(BACKL, 0);
}

void dotBrightTick() {
  if (curStatus == 1) {
    setPWM(DOT, dotMaxBright); // Секундная точка горит в статическом режиме при показе дня и месяца
  }
  else if (curMode == STOPWATCH_READY || curMode == STOPWATCH_RUN || curMode == STOPWATCH_STOP) {
    setPWM(DOT, dotMaxBright); // Секундная точка горит в статическом режиме в режиме секундомера
  }
  else if (curStatus == 2) {
    digitalWrite(DOT, 0); // Секундная точка не горит при показе температуры
  }
  else if (dotBrightFlag && dotBrightTimer.isReady()) { // Мигание секундной точки
    if (dotBrightDirection) {
      dotBrightCounter += dotBrightStep;
      if (dotBrightCounter >= dotMaxBright) {
        dotBrightDirection = false;
        dotBrightCounter = dotMaxBright;
      }
    }
    else {
      dotBrightCounter -= dotBrightStep;
      if (dotBrightCounter <= 0) {
        dotBrightDirection = true;
        dotBrightCounter = 0;
        dotBrightFlag = false;
      }
    }
    setPWM(DOT, getPWM_CRT(dotBrightCounter));
  }
}

void changeBright()
{
  if (NIGHT_LIGHT == 1) {  // Установка яркости всех светилок от времени суток
    if (hrs >= NIGHT_START && hrs < NIGHT_END) {
      indiMaxBright = INDI_BRIGHT_N;
      dotMaxBright = DOT_BRIGHT_N;
      backlMaxBright = BACKL_BRIGHT_N;
    }
    else {
      indiMaxBright = INDI_BRIGHT;
      dotMaxBright = DOT_BRIGHT;
      backlMaxBright = BACKL_BRIGHT;
    }
  }

   // Установить яркость индикаторов
  for (byte i = 0; i < 4; i++) {
    indiDimm[i] = indiMaxBright;
  }

  // Расчёт шага яркости точки
  dotBrightStep = ceil((float)dotMaxBright * 2 / DOT_TIME * DOT_TIMER);
  if (dotBrightStep == 0) {
      dotBrightStep = 1;
  }

  // Дыхание подсветки
  if (backlMaxBright > 0) {
    backlBrightTimer.setInterval((float)BACKL_STEP / backlMaxBright / 2 * BACKL_TIME);
  }

  // Счетчик уровня яркости для первого эффекта смены времени
  indiBrightCounter = indiMaxBright;
}
