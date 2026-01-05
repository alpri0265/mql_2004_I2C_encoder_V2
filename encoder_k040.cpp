 #include "encoder_k040.h"

void EncoderK040::begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn) {
  _pA = pinA; _pB = pinB; _pBtn = pinBtn;
  pinMode(_pA, INPUT_PULLUP);
  pinMode(_pB, INPUT_PULLUP);
  pinMode(_pBtn, INPUT_PULLUP);
  _lastA = digitalRead(_pA);
}

EncoderEvents EncoderK040::poll() {
  EncoderEvents ev = {0, false};
  
  // Прямое чтение из базового скетча
  uint8_t curA = digitalRead(_pA);
  if (curA != _lastA) {
    if (digitalRead(_pB) != curA) ev.step = 1;
    else ev.step = -1;
    _lastA = curA;
  }

  // Кнопка БЕЗ сохранения состояния — просто проверяем нажатие
  if (digitalRead(_pBtn) == LOW) {
    ev.click = true;
  }
  
  return ev;
}