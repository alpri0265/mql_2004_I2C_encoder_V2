#ifndef ENCODER_K040_H
#define ENCODER_K040_H
#include <Arduino.h>

struct EncoderEvents {
  int8_t  step;
  bool    click;
};

class EncoderK040 {
public:
  void begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn);
  EncoderEvents poll();
private:
  uint8_t _pA, _pB, _pBtn, _lastA;
  uint32_t _timer = 0;
};
#endif