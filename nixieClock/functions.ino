/**
 * Функция очистки индикаторов (эффект "прожига")
 * Последовательно перебирает все цифры на всех индикаторах
 **/
void burnIndicators() {
  // Внешний цикл - количество повторений эффекта
  for (byte k = 0; k < BURN_LOOPS; k++) {
    // Средний цикл - количество шагов изменения цифр
    for (byte d = 0; d < 10; d++) {
      // Внутренний цикл - обработка каждого индикатора
      for (byte i = 0; i < 4; i++) {
        indiDigits[i]--;
        // Зацикливание значений (9 -> 8 -> ... -> 0 -> 9)
        if (indiDigits[i] < 0) indiDigits[i] = 9;
      }
      delay(BURN_TIME);  // Задержка между шагами
    }
  }
}

/**
 * Функция обработки глюков (случайные мерцания индикаторов)
 * Вызывается в основном цикле программы
 **/
void glitchTick() {
  // Проверка условий для начала глюка
  if (!glitchFlag && secs > 7 && secs < 40) {
    if (glitchTimer.isReady()) {
      // Инициализация нового глюка
      glitchFlag = true;
      indiState = 0;  // Начинаем с выключенного состояния
      glitchCounter = 0;
      glitchMax = random(3, 12);  // Случайное количество мерцаний
      glitchIndic = random(0, 4);  // Случайный индикатор
      glitchTimer.setInterval(random(1, 6) * 30);  // Случайный интервал
    }
  }
  // Обработка активного глюка
  else if (glitchFlag && glitchTimer.isReady()) {
    // Установка яркости индикатора (0 или максимальная)
    indiDimm[glitchIndic] = indiState * indiMaxBright;
    indiState = !indiState;  // Переключение состояния
    glitchTimer.setInterval(random(1, 6) * 25);  // Новый интервал
    glitchCounter++;
    
    // Проверка завершения глюка
    if (glitchCounter > glitchMax) {
      glitchTimer.setInterval(random(GLITCH_MIN * 1000L, GLITCH_MAX * 1000L));
      glitchFlag = false;
      indiDimm[glitchIndic] = indiMaxBright;  // Восстановление яркости
    }
  }
}

/**
 * Обработчик прерывания таймера 2 (динамическая индикация)
 * Управляет последовательным отображением цифр на индикаторах
 **/
ISR(TIMER2_COMPA_vect) {
  // Увеличение счетчика текущего индикатора
  indiCounter[curIndi]++;
  
  // Проверка достижения порога диммирования
  if (indiCounter[curIndi] >= indiDimm[curIndi]) {
    setPin(opts[curIndi], 0);  // Выключение индикатора
  }

  // Проверка необходимости переключения на следующий индикатор
  if (indiCounter[curIndi] > 25) {
    indiCounter[curIndi] = 0;  // Сброс счетчика
    
    // Переключение на следующий индикатор (с закольцовыванием)
    if (++curIndi >= 4) curIndi = 0;

    // Отправка цифры на текущий индикатор
    if (indiDimm[curIndi] > 0) {
      byte thisDig = digitMask[indiDigits[curIndi]];
      setPin(DECODER3, bitRead(thisDig, 0));
      setPin(DECODER1, bitRead(thisDig, 1));
      setPin(DECODER0, bitRead(thisDig, 2));
      setPin(DECODER2, bitRead(thisDig, 3));
      setPin(opts[curIndi], anodeStates[curIndi]);  // Включение анода
    }
  }
}

/**
 * Функция отправки времени на индикаторы
 * @param hours - часы (0-23)
 * @param minutes - минуты (0-59)
 **/
void sendTime(byte hours, byte minutes) {
  indiDigits[0] = hours / 10;    // Десятки часов
  indiDigits[1] = hours % 10;    // Единицы часов
  indiDigits[2] = minutes / 10;  // Десятки минут
  indiDigits[3] = minutes % 10;  // Единицы минут
}

/**
 * Функция обновления массива нового времени
 * Используется для эффектов переключения цифр
 **/
void setNewTime() {
  newTime[0] = (byte)hrs / 10;
  newTime[1] = (byte)hrs % 10;
  newTime[2] = (byte)mins / 10;
  newTime[3] = (byte)mins % 10;
}

/**
 * Функция расчета и обновления времени
 **/
byte minsCount = 0;
void calculateTime() {
  // Проверка текущего режима
  if (curMode != CLOCK) return;

  // Переключение состояния точки (мигание)
  dotFlag = !dotFlag;
  
  if (dotFlag) {
    // Инициализация яркости точки
    dotBrightFlag = true;
    dotBrightDirection = true;
    dotBrightCounter = 0;
    
    // Увеличение счетчика секунд
    secs++;

    // Проверка необходимости очистки индикаторов
    if (secs > 19 && secs <= 25 && mins % BURN_PERIOD == 0) {
      burnIndicators();  // Эффект очистки
    }
    
    // Переключение режимов отображения
    if (secs > 39 && secs <= 45 && COUNTRY) {
      curStatus = 1;  // Режим отображения даты
    }
    else if (secs > 45 && secs <= 50 && TEMP) {
      curStatus = 2;  // Режим отображения температуры
    }
    else {
      curStatus = 0;  // Основной режим (время)
    }

    // Проверка завершения минуты
    if (secs > 59) {
      // Переключение эффектов (если включен режим loop)
      if (EFFECT_LOOP) {
        if (++FLIP_EFFECT >= FLIP_EFFECT_NUM) FLIP_EFFECT = 1;
        flipTimer.setInterval(FLIP_SPEED[FLIP_EFFECT]);
      }

      newTimeFlag = true;  // Флаг обновления времени
      secs = 0;  // Сброс секунд
      mins++;  // Увеличение минут
      minsCount++;  // Счетчик для синхронизации

      // Синхронизация с RTC (каждые 10 минут)
      if (minsCount > 8 && (byte)mins % 10 == 7) {
        minsCount = 0;  // Сброс счетчика
        
        // Чтение времени из RTC
        DateTime now = rtc.now();
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
        Year = now.year();
        Month = now.month();
        Day = now.day();
        EEPROM.get(3, DayWeek);  // Чтение дня недели
        
        showRTC();  // Отладочный вывод
      }
    }
    
    // Проверка завершения часа
    if (mins > 59) {
      
      mins = 0;  // Сброс минут
      hrs++;  // Увеличение часов

      // Проверка завершения дня
      if (hrs > 23) {
        hrs = 0;  // Сброс часов
        Day++;  // Увеличение дня
        DayWeek++;
        
        // Коррекция дня недели
        if (DayWeek > 7) DayWeek = 1;
        EEPROM.put(3, DayWeek);  // Сохранение в EEPROM
      }
      
      changeBright(); // Обновление яркости

      // Проверка завершения месяца
      if (Day > DaysInMonth[Month - 1]) {
        Day = 1;  // Сброс дня
        Month++;  // Увеличение месяца
      }
      
      // Проверка завершения года
      if (Month > 12) {
        Month = 1;  // Сброс месяца
        Year++;  // Увеличение года
      }
    }
  }
  
  // Обновление массива нового времени
  if (newTimeFlag) setNewTime();
}

/**
 * Функция отладочного вывода времени из RTC
 **/
void showRTC() {
  if (DEBUG) {
    DateTime now = rtc.now();

    // Форматированный вывод даты и времени
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(DayWeek, DEC);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
}

/**
 * Функция анимации "паровозик" для отображения дня недели
 * Последовательно выключает индикаторы
 **/
void DayTrain() {
  if (!flipInit) {
    // Инициализация анимации
    anodeStates[currentLamp++] = 0;
    flipInit = true;
    currentLamp = 0;
    flipTimer.reset();
    flipTimer.setInterval(30);  // Интервал анимации
  }
  
  if (flipTimer.isReady()) {
    // Последовательное выключение индикаторов
    anodeStates[currentLamp++] = 0;
    
    // Проверка завершения цикла
    if (currentLamp >= 4) {
      currentLamp = 0;
      flipInit = false;
    }
  }
}