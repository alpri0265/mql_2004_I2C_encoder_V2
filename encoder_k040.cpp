#include "encoder_k040.h"
#include <EncButton.h>

// Храним реальный тип здесь, чтобы .h оставался лёгким
static inline EncButton* EB(void* p) { return reinterpret_cast<EncButton*>(p); }

void EncoderK040::begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn) {
  _pA = pinA; _pB = pinB; _pBtn = pinBtn;

  // Освобождаем, если begin() вызывается повторно
  if (_eb) {
    delete EB(_eb);
    _eb = nullptr;
  }

  // KY-040 с RC-цепочкой и подтяжками на модуле:
  // A/B читаем как INPUT (внешние подтяжки), BTN — INPUT_PULLUP (кнопка на GND)
  _eb = new EncButton(_pA, _pB, _pBtn, INPUT, INPUT_PULLUP);

  // Тип энкодера: 4 фазы = 1 щелчок, активный LOW (в покое A=1, B=1)
  EB(_eb)->setEncType(EB_STEP4_LOW);

  // Кнопка: нажатие = LOW (замыкание на GND)
  EB(_eb)->setBtnLevel(LOW);

  // ISR режим здесь НЕ используем, потому что на Uno D3 занят под STEP (PIN_STEP = 3).
  // Если вы перенесёте STEP на другой пин, можно будет включить ISR-режим.
}

EncoderEvents EncoderK040::poll() {
  EncoderEvents ev;
  if (!_eb) return ev;

  EB(_eb)->tick();

  if (EB(_eb)->right()) ev.step = 1;
  else if (EB(_eb)->left()) ev.step = -1;

  ev.click = EB(_eb)->click();
  ev.hold  = EB(_eb)->hold();

  return ev;
}
