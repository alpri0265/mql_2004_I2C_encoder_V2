#pragma once
#include <Arduino.h>

// Структура событий ввода (энкодер + кнопки)
struct InputEvents {
  int8_t  encStep     = 0;
  bool    encClick    = false;
  bool    encLong     = false;
  bool    menuClick   = false;
  bool    startClick  = false;
};

void inputBegin();
void inputPoll(InputEvents &ev);

// Функции для потенциометра
void potSetFilterN(uint8_t N);
uint16_t potGetAvgAdc();