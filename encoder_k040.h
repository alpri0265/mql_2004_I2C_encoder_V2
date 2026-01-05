#pragma once
#include <Arduino.h>

// Локальная структура событий энкодера (не зависит от других заголовков проекта)
struct EncoderEvents {
  int8_t step = 0;     // +1 / -1 за один "щелчок"
  bool   click = false; // короткое нажатие (OK)
  bool   hold = false;  // длинное нажатие (MENU/BACK)
};

class EncButton;  // forward

// Обёртка над EncButton v3 для KY-040.
// Важно: наружу отдаём только события, логика меню/насоса не меняется.
class EncoderK040 {
public:
  void begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn);
  EncoderEvents poll();

private:
  EncButton* _eb = nullptr;
};
