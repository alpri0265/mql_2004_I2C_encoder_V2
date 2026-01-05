#ifndef ENCODER_K040_H
#define ENCODER_K040_H

#include <Arduino.h>
#include "types.h"      // EncoderEvents

// Обёртка над EncButton v3 (энкодер KY-040).
// Важно: логика меню/насоса не меняется — наружу отдаём только события (step/click/hold).
class EncoderK040 {
public:
  void begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn);
  EncoderEvents poll();

private:
  uint8_t _pA = 0, _pB = 0, _pBtn = 0;
  void* _eb = nullptr;   // EncButton* (чтобы не тянуть заголовок EncButton в .h)
};

#endif
