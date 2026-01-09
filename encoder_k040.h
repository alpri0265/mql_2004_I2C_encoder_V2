#pragma once
#include <Arduino.h>

struct EncoderEvents {
  int8_t step = 0;      // +1 / -1 за один щелчок
  bool   click = false; // короткое нажатие (OK)
  bool   hold = false;  // длинное нажатие (MENU/BACK)
};

class EncoderK040 {
public:
  // pinA = D2, pinB = D3 (для UNO/Micro это INT0/INT1), pinBtn = A3
  void begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn);
  EncoderEvents poll();

private:
  uint8_t _pinA = 0, _pinB = 0;
  bool _inited = false;
};
