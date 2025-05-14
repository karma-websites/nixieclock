/**
 * Функция обработки эффектов переключения цифр на индикаторах
 * Вызывается при необходимости обновления времени
 **/
void flipTick() {
  // Эффект 0 - отсутствие эффекта (простое отображение)
  if (FLIP_EFFECT == 0) {
    if (hrs > 12) {
      sendTime(hrs - (12 * CLOCK_IND), mins);  // 12-часовой формат
    } else {
      sendTime(hrs, mins);  // 24-часовой формат
    }
    newTimeFlag = false;
  }
  
  // Эффект 1 - плавное угасание и появление цифр
  else if (FLIP_EFFECT == 1) {
    if (!flipInit) {
      // Инициализация эффекта
      flipInit = true;
      
      // Определение, какие цифры изменились и требуют анимации
      for (byte i = 0; i < 4; i++) {
        flipIndics[i] = (indiDigits[i] != newTime[i]);
      }
    }

    if (flipTimer.isReady()) {
      if (!indiBrightDirection) {
        // Фаза уменьшения яркости
        indiBrightCounter--;
        if (indiBrightCounter <= 0) {
          // Переход к фазе увеличения яркости
          indiBrightDirection = true;
          indiBrightCounter = 0;
          
          // Установка новых значений времени
          if (hrs > 12) {
            sendTime(hrs - (12 * CLOCK_IND), mins);
          }
          else {
            sendTime(hrs, mins);
          }
        }
      } else {
        // Фаза увеличения яркости
        indiBrightCounter++;
        if (indiBrightCounter >= indiMaxBright) {
          // Завершение эффекта
          indiBrightDirection = false;
          indiBrightCounter = indiMaxBright;
          flipInit = false;
          newTimeFlag = false;
        }
      }
      
      // Применение яркости к изменяемым цифрам
      for (byte i = 0; i < 4; i++) {
        if (flipIndics[i]) indiDimm[i] = indiBrightCounter;
      }
    }
  }
  
  // Эффект 2 - перемотка цифр по порядку (0-9)
  else if (FLIP_EFFECT == 2) {
    if (!flipInit) {
      // Инициализация эффекта
      flipInit = true;
      
      // Определение, какие цифры изменились
      for (byte i = 0; i < 4; i++) {
        flipIndics[i] = (indiDigits[i] != newTime[i]);
      }
    }

    if (flipTimer.isReady()) {
      byte unchangedCount = 0;
      
      // Обработка каждой цифры
      for (byte i = 0; i < 4; i++) {
        if (flipIndics[i]) {
          // Прокрутка цифры
          indiDigits[i]--;
          if (indiDigits[i] < 0) {
            indiDigits[i] = 9;
          }
          
          // Проверка достижения целевого значения
          if (indiDigits[i] == newTime[i]) {
            flipIndics[i] = false;
          }
        } else {
          unchangedCount++;
        }
      }
      
      // Проверка завершения эффекта
      if (unchangedCount == 4) {
        flipInit = false;
        newTimeFlag = false;
      }
    }
  }
  
  // Эффект 3 - перемотка по порядку катодов в лампе
  else if (FLIP_EFFECT == 3) {
    if (!flipInit) {
      // Инициализация эффекта
      flipInit = true;
      
      // Определение начальных и конечных позиций катодов
      for (byte i = 0; i < 4; i++) {
        if (indiDigits[i] != newTime[i]) {
          flipIndics[i] = true;
          
          // Поиск позиций в таблице катодов
          for (byte c = 0; c < 10; c++) {
            if (cathodeMask[c] == indiDigits[i]) {
              startCathode[i] = c;
            }
            if (cathodeMask[c] == newTime[i]) {
              endCathode[i] = c;
            }
          }
        } else {
          flipIndics[i] = false;
        }
      }
    }

    if (flipTimer.isReady()) {
      byte unchangedCount = 0;
      
      // Обработка каждой цифры
      for (byte i = 0; i < 4; i++) {
        if (flipIndics[i]) {
          // Прокрутка к целевому значению
          if (startCathode[i] > endCathode[i]) {
            startCathode[i]--;
          } else if (startCathode[i] < endCathode[i]) {
            startCathode[i]++;
          } else {
            flipIndics[i] = false;
          }
          
          // Установка текущего значения цифры
          indiDigits[i] = cathodeMask[startCathode[i]];
        } else {
          unchangedCount++;
        }
      }
      
      // Проверка завершения эффекта
      if (unchangedCount == 4) {
        flipInit = false;
        newTimeFlag = false;
      }
    }
  }
  
  // Эффект 4 - "поезд" (последовательное перемещение цифр)
  else if (FLIP_EFFECT == 4) {
    if (!flipInit) {
      // Инициализация эффекта
      flipInit = true;
      currentLamp = 0;
      trainLeaving = true;
      flipTimer.reset();
    }
    
    if (flipTimer.isReady()) {
      if (trainLeaving) {
        // Фаза "ухода" цифр
        for (byte i = 3; i > currentLamp; i--) {
          indiDigits[i] = indiDigits[i - 1];
        }
        anodeStates[currentLamp] = 0;
        currentLamp++;
        
        // Проверка завершения фазы
        if (currentLamp >= 4) {
          trainLeaving = false;
          currentLamp = 0;
        }
      } else {
        // Фаза "прихода" новых цифр
        for (byte i = currentLamp; i > 0; i--) {
          indiDigits[i] = indiDigits[i - 1];
        }
        indiDigits[0] = newTime[3 - currentLamp];
        anodeStates[currentLamp] = 1;
        currentLamp++;
        
        // Проверка завершения эффекта
        if (currentLamp >= 4) {
          flipInit = false;
          newTimeFlag = false;
        }
      }
    }
  }
  
  // Эффект 5 - "резинка" (сложный последовательный эффект)
  else if (FLIP_EFFECT == 5) {
    if (!flipInit) {
      // Инициализация эффекта
      flipInit = true;
      flipEffectStages = 0;
      flipTimer.reset();
    }
    
    if (flipTimer.isReady()) {
      // Обработка каждой стадии эффекта
      switch (flipEffectStages++) {
        // Стадии 1-10: "схлопывание" цифр
        case 1: anodeStates[3] = 0; break;
        case 2:
          anodeStates[2] = 0;
          indiDigits[3] = indiDigits[2];
          anodeStates[3] = 1;
          break;
        case 3: anodeStates[3] = 0; break;
        case 4:
          anodeStates[1] = 0;
          indiDigits[2] = indiDigits[1];
          anodeStates[2] = 1;
          break;
        case 5:
          anodeStates[2] = 0;
          indiDigits[3] = indiDigits[1];
          anodeStates[3] = 1;
          break;
        case 6: anodeStates[3] = 0; break;
        case 7:
          anodeStates[0] = 0;
          indiDigits[1] = indiDigits[0];
          anodeStates[1] = 1;
          break;
        case 8:
          anodeStates[1] = 0;
          indiDigits[2] = indiDigits[0];
          anodeStates[2] = 1;
          break;
        case 9:
          anodeStates[2] = 0;
          indiDigits[3] = indiDigits[0];
          anodeStates[3] = 1;
          break;
        case 10: anodeStates[3] = 0; break;
          
        // Стадии 11-14: появление первой новой цифры
        case 11:
          indiDigits[0] = newTime[3];
          anodeStates[0] = 1;
          break;
        case 12:
          anodeStates[0] = 0;
          indiDigits[1] = newTime[3];
          anodeStates[1] = 1;
          break;
        case 13:
          anodeStates[1] = 0;
          indiDigits[2] = newTime[3];
          anodeStates[2] = 1;
          break;
        case 14:
          anodeStates[2] = 0;
          indiDigits[3] = newTime[3];
          anodeStates[3] = 1;
          break;
          
        // Стадии 15-17: появление второй новой цифры
        case 15:
          indiDigits[0] = newTime[2];
          anodeStates[0] = 1;
          break;
        case 16:
          anodeStates[0] = 0;
          indiDigits[1] = newTime[2];
          anodeStates[1] = 1;
          break;
        case 17:
          anodeStates[1] = 0;
          indiDigits[2] = newTime[2];
          anodeStates[2] = 1;
          break;
          
        // Стадии 18-19: появление третьей новой цифры
        case 18:
          indiDigits[0] = newTime[1];
          anodeStates[0] = 1;
          break;
        case 19:
          anodeStates[0] = 0;
          indiDigits[1] = newTime[1];
          anodeStates[1] = 1;
          break;
          
        // Стадия 20: появление последней новой цифры
        case 20:
          indiDigits[0] = newTime[0];
          anodeStates[0] = 1;
          break;
          
        // Завершение эффекта
        case 21:
          flipInit = false;
          newTimeFlag = false;
          break;
      }
    }
  }
}