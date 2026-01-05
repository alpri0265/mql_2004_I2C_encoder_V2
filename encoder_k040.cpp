#include "encoder_k040.h"
#include <EncButton.h>

void EncoderK040::begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn) {
  // Пересоздаём объект, если begin() вызван повторно
  if (_eb) {
    delete _eb;
    _eb = nullptr;
  }

  // KY-040 с RC-цепочкой и подтяжками на модуле:
  // A/B -> INPUT (внешние подтяжки), BTN -> INPUT_PULLUP (кнопка на GND)
  _eb = new EncButton(pinA, pinB, pinBtn, INPUT, INPUT_PULLUP);

  // Тип энкодера: 4 фазы = 1 щелчок, активный LOW (A=B=1 в покое)
  _eb->setEncType(EB_STEP4_LOW);

  // Кнопка: активный уровень LOW (замыкает на GND)
  _eb->setBtnLevel(LOW);
}

EncoderEvents EncoderK040::poll() {
  EncoderEvents ev;
  if (!_eb) return ev;

  _eb->tick();

  if (_eb->right()) ev.step = +1;
  else if (_eb->left()) ev.step = -1;

  ev.click = _eb->click();
  ev.hold  = _eb->hold();

  return ev;
}
