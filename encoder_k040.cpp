#include "encoder_k040.h"
#include <EncButton.h>

// Глобальный объект EncButton (нужен для ISR)
static EncButton eb(2, 3, A3, INPUT, INPUT_PULLUP);  // значения переопределим в begin()
static bool isrAttached = false;

static void encISR() {
  eb.tickISR();
}

void EncoderK040::begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn) {
  _pinA = pinA;
  _pinB = pinB;

  // Пересоздаем объект, чтобы применились пины/режимы
  eb = EncButton(pinA, pinB, pinBtn, INPUT, INPUT_PULLUP);

  // KY-040 с RC и подтяжками на модуле: активный LOW, 4 фазы = 1 щелчок
  eb.setEncType(EB_STEP4_LOW);
  eb.setBtnLevel(LOW);

  // Включаем ISR-режим чтения энкодера
  eb.setEncISR(true);

  // Подключаем прерывания один раз
  if (!isrAttached) {
    attachInterrupt(digitalPinToInterrupt(_pinA), encISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pinB), encISR, CHANGE);
    isrAttached = true;
  }

  _inited = true;
}

EncoderEvents EncoderK040::poll() {
  EncoderEvents ev;
  if (!_inited) return ev;

  // Важно: tick() обязателен — он выдаёт события (left/right/click/hold)
  eb.tick();

  if (eb.right()) ev.step = +1;
  else if (eb.left()) ev.step = -1;

  ev.click = eb.click();
  ev.hold  = eb.hold();
  return ev;
}
