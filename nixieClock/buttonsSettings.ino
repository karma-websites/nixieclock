/**
 * Функция обновления состояния индикации в зависимости от текущего режима
 * Вызывается по таймеру blinkTimer
 **/
void settingsTick() {
  if (blinkTimer.isReady()) {
    // Переключение состояния лампы (мигание)
    lampState = !lampState;
    
    // По умолчанию все аноды включены
    anodeStates[0] = anodeStates[1] = anodeStates[2] = anodeStates[3] = 1;

    switch (curMode) {
      case CLOCK:  // Основной режим часов
        switch (curStatus) {
          case 0:  // Режим отображения времени
            if (!newTimeFlag) {
              if (hrs > 12) {
                sendTime(hrs - (12 * CLOCK_IND), mins);  // 12-часовой формат
              } else {
                sendTime(hrs, mins);  // 24-часовой формат
              }
            }
            break;
            
          case 1:  // Режим отображения даты
            if (COUNTRY == 1) {
              sendTime(Day, Month);  // Формат День.Месяц (Россия)
            } else {
              sendTime(Month, Day);  // Формат Месяц.День (США)
            }
            break;
            
          case 2:  // Режим отображения температуры
            // Отключаем крайние индикаторы (для отображения "_tt_")
            anodeStates[0] = anodeStates[3] = 0;
            
            // Запрос температуры с датчика
            sensor.requestTemp();
            
            // Конвертация температуры в нужные единицы
            if (TEMP == 1) {
              meas = sensor.getTemp() + temp_delta;  // градусы Цельсия
            }
            if (TEMP == 2) {
              meas = sensor.getTemp() * 1.8 + 32.0 + temp_delta;  // градусы Фаренгейта
            }
            
            // Установка значений для отображения
            indiDigits[1] = (byte)meas / 10;
            indiDigits[2] = (byte)meas % 10;
            break;
        }
        break;

      // Далее идут режимы настройки
      case MONTH:  // Настройка числа месяца
        if (!lampState && COUNTRY == 1) {
          anodeStates[2] = anodeStates[3] = 0;  // Мигание правых индикаторов
        } else if (!lampState && COUNTRY != 1) {
          anodeStates[0] = anodeStates[1] = 0;  // Мигание левых индикаторов
        }
        
        // Отображение даты в зависимости от страны
        if (COUNTRY == 1) sendTime(changeDay, changeMonth);  // Россия
        else sendTime(changeMonth, changeDay);  // США
        break;

      case DAY:  // Настройка дня
        if (!lampState && (COUNTRY == 1)) {
          anodeStates[0] = anodeStates[1] = 0;
        } else if (!lampState && (COUNTRY != 1)) {
          anodeStates[2] = anodeStates[3] = 0;
        }

        if (COUNTRY == 1) sendTime(changeDay, changeMonth);
        else sendTime(changeMonth, changeDay);
        break;

      case DAYW:  // Настройка дня недели
        if (!lampState) {
          // Мигание всех индикаторов
          anodeStates[0] = anodeStates[1] = anodeStates[2] = anodeStates[3] = 0;
        }
        // На всех индикаторах отображается номер дня недели
        indiDigits[0] = indiDigits[1] = indiDigits[2] = indiDigits[3] = changeDayWeek;
        break;

      case SET_TEMP:  // Настройка коррекции температуры
        anodeStates[0] = anodeStates[3] = 0;  // Формат "_tt_"
        sensor.requestTemp();
        if (TEMP == 1) {
          meas = sensor.getTemp() + temp_delta;
        }
        if (TEMP == 2) {
          meas = sensor.getTemp() * 1.8 + 32.0 + temp_delta;
        }
        indiDigits[1] = (byte)meas / 10;
        indiDigits[2] = (byte)meas % 10;
        break;

      case HRS:  // Настройка часов
        if (!lampState) {
          anodeStates[0] = anodeStates[1] = 0;  // Мигание часов
        }
        sendTime(changeHrs, changeMins);
        break;
        
      case MIN:  // Настройка минут
        if (!lampState) {
          anodeStates[2] = anodeStates[3] = 0;  // Мигание минут
        }
        sendTime(changeHrs, changeMins);
        break;
      
      // Режимы секундомера
      case STOPWATCH_READY:  // Ожидание старта
        if (lampState) {
          sendTime(0, 0);  // Отображение 00:00
        } else {
          // Все индикаторы выключены (мигание)
          anodeStates[0] = anodeStates[1] = anodeStates[2] = anodeStates[3] = 0;
        }
        break;

      case STOPWATCH_RUN:  // Секундомер работает
      {
        // Расчет текущего времени
        stopwatchCur = stopwatchPrev + millis() - stopwatchStart;
        byte seconds = (stopwatchCur / 1000) % 60;
        byte minutes = stopwatchCur / 60000;
        
        // Проверка на достижение максимума (59:59)
        if (minutes == 59 && seconds == 59) {
          curMode = STOPWATCH_STOP;
          stopwatchFinish = true;
        }
        sendTime(minutes, seconds);
        break;
      }

      case STOPWATCH_STOP:  // Секундомер остановлен
      {
        // Отображение зафиксированного времени
        byte seconds = (stopwatchCur / 1000) % 60;
        byte minutes = stopwatchCur / 60000;
        sendTime(minutes, seconds);
        break;
      }
    }
  }
}

/**
 * Обработка нажатий кнопок
 **/
void buttonsTick() {
  // Обновление состояний кнопок
  btnSet.tick();
  btnL.tick();
  btnR.tick();

  // Обработка клика по кнопке SET (левая кнопка)
  if (btnSet.isClick() && !stopwatchRun) {
    // Переключение между режимами настройки
    if (++curMode >= 7) curMode = 0;
    
    switch (curMode) {
      case CLOCK:  // Выход из режима настройки - сохранение значений
        Month = changeMonth;
        Day = changeDay;
        DayWeek = changeDayWeek;
        EEPROM.put(3, DayWeek);
        hrs = changeHrs;
        mins = changeMins;
        secs = 0;
        rtc.adjust(DateTime(Year, Month, Day, hrs, mins, 0));
        showRTC();  // Отладочный вывод
        changeBright();
        break;

      // Инициализация значений для редактирования
      case MONTH:
        changeMonth = Month;
        changeDay = Day;
        break;
      case DAY: // проблемы
        changeDay = Day;
        break;
      case DAYW:
        changeDayWeek = DayWeek;
        break;
      case SET_TEMP:
        if (TEMP == 0) { // Пропуск, если датчик не подключен
          curMode = HRS;
          changeHrs = hrs;
          changeMins = mins;
        }
        break;
      case HRS:
        changeHrs = hrs;
        changeMins = mins;
        break;
      case MIN:
        changeMins = mins;
        break;
    }
  }

  // Обработка клика по кнопке R (правая кнопка)
  if (btnR.isClick()) {
    switch (curMode) {
      // Вход в режим секундомера
      case CLOCK:
        curMode = STOPWATCH_READY;
        stopwatchRun = true;
        break;

      // Увеличение значений в режимах настройки
      case MONTH:
        changeMonth++;
        if (changeMonth > 12) changeMonth = 1;
        break;
      case DAY:
        changeDay++;
        if (changeDay > DaysInMonth[(changeMonth - 1)]) changeDay = 1;
        break;
      case DAYW:
        changeDayWeek++;
        if (changeDayWeek > 7) changeDayWeek = 1;
        break;
      case SET_TEMP:
        temp_delta++;
        break;
      case HRS:
        changeHrs++;
        if (changeHrs > 23) changeHrs = 0;
        break;
      case MIN:
        changeMins++;
        if (changeMins > 59) changeMins = 0;
        break;

      // Управление секундомером
      case STOPWATCH_READY:  // Старт
        curMode = STOPWATCH_RUN;
        stopwatchStart = millis();
        break;
      case STOPWATCH_RUN:  // Стоп
        curMode = STOPWATCH_STOP;
        stopwatchPrev = stopwatchCur;
        break;
      case STOPWATCH_STOP:  // Продолжение/сброс
        if (stopwatchFinish) {
          curMode = STOPWATCH_READY;
          stopwatchPrev = 0;
          stopwatchFinish = false;
        } else {
          curMode = STOPWATCH_RUN;
          stopwatchStart = millis();
        }
        break;
    }
  }

  // Обработка удержания кнопки R
  if (btnR.isHolded() && stopwatchRun) { // Выход из секундомера в режим часов
    curMode = CLOCK;
    stopwatchPrev = 0;
    stopwatchRun = false;

    DateTime now = rtc.now();
    hrs = now.hour();
    mins = now.minute();
    secs = now.second();
    sendTime(hrs, mins);
  }

  // Обработка клика по кнопке L (центральная кнопка)
  if (btnL.isClick() && !stopwatchRun) {
    switch (curMode) {
      case CLOCK:  // Переключение режимов подсветки
        if (++BACKL_MODE >= 3) BACKL_MODE = 0;
        EEPROM.put(1, BACKL_MODE);
        break;
        
      // Уменьшение значений в режимах настройки
      case MONTH:
        changeMonth--;
        if (changeMonth < 1) changeMonth = 12;
        break;
      case DAY:
        changeDay--;
        if (changeDay < 1) changeDay = DaysInMonth[changeMonth - 1];
        break;
      case DAYW:
        changeDayWeek--;
        if (changeDayWeek < 1) changeDayWeek = 7;
        break;
      case SET_TEMP:
        temp_delta--;
        break;
      case HRS:
        changeHrs--;
        if (changeHrs < 0) changeHrs = 23;
        break;
      case MIN:
        changeMins--;
        if (changeMins < 0) changeMins = 59;
        break;
    }
  }

  // Обработка удержания кнопки L - включение/выключение глюков
  if (btnL.isHolded() && curMode == CLOCK) {
    GLITCH_ALLOWED = !GLITCH_ALLOWED;
    if (GLITCH_ALLOWED) sendTime(11, 11);
    else sendTime(0, 0);
    EEPROM.put(2, GLITCH_ALLOWED);
  }
}