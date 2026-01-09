#include "encoder_k040.h"
#include "config.h"
#include <Arduino.h>

// ======= ENCODER (A=D2, B=D3) ISR decoder =======
// Таблица переходов (Gray) 00/01/10/11 -> delta
// index = (prev<<2) | curr
static const int8_t kTrans[16] = {
  0, -1, +1,  0,
 +1,  0,  0, -1,
 -1,  0,  0, +1,
  0, +1, -1,  0
};

static volatile int16_t isrEdges = 0;     // накопление "полушагов" (edges)
static volatile uint8_t prevAB = 0;      // 2-битное предыдущее состояние
static volatile uint32_t lastEdgeUs = 0; // защита от дребезга по времени

// Пины энкодера (сохраняем для чтения в ISR)
static uint8_t encPinA = 2;
static uint8_t encPinB = 3;

// Портируемое чтение состояния энкодера (работает на AVR и ARM)
static inline uint8_t readAB_portable() {
  uint8_t a = digitalRead(encPinA) ? 1 : 0;
  uint8_t b = digitalRead(encPinB) ? 1 : 0;
  return (a << 1) | b;  // AB in bits: A as MSB, B as LSB
}

static void encISR() {
  uint32_t us = micros();
  if ((uint32_t)(us - lastEdgeUs) < ENC_MIN_EDGE_US) return;
  lastEdgeUs = us;

  uint8_t ab = readAB_portable();
  uint8_t idx = (prevAB << 2) | ab;
  prevAB = ab;

  int8_t d = kTrans[idx];
  if (d) isrEdges += d;
}

// ======= BUTTON (BTN=A3) debounce + click/hold =======
static uint8_t pinBtn = A3;

static bool     btnStable = true;     // HIGH = released
static bool     btnLastRead = true;
static uint32_t btnLastChangeMs = 0;
static uint32_t btnPressMs = 0;
static bool     holdFired = false;

static void btnInit() {
  pinMode(pinBtn, INPUT_PULLUP);
  btnStable = digitalRead(pinBtn);
  btnLastRead = btnStable;
  btnLastChangeMs = millis();
  btnPressMs = 0;
  holdFired = false;
}

static void btnPoll(bool &click, bool &hold) {
  click = false;
  hold  = false;

  bool r = digitalRead(pinBtn); // HIGH released, LOW pressed

  if (r != btnLastRead) {
    btnLastRead = r;
    btnLastChangeMs = millis();
  }

  // debounce
  if ((uint32_t)(millis() - btnLastChangeMs) >= ENC_BTN_DEBOUNCE_MS) {
    if (btnStable != btnLastRead) {
      bool prev = btnStable;
      btnStable = btnLastRead;

      if (prev == HIGH && btnStable == LOW) {        // press
        btnPressMs = millis();
        holdFired = false;
      } else if (prev == LOW && btnStable == HIGH) { // release
        // short click if hold wasn't fired
        if (!holdFired) click = true;
        btnPressMs = 0;
      }
    }
  }

  // hold one-shot
  if (btnStable == LOW && !holdFired) {
    if ((uint32_t)(millis() - btnPressMs) >= ENC_BTN_LONG_MS) {
      holdFired = true;
      hold = true;
    }
  }
}

// ======= EncoderK040 implementation =======
static bool isrAttached = false;

void EncoderK040::begin(uint8_t pinA, uint8_t pinB, uint8_t pinBtn_) {
  // Сохраняем пины для использования в ISR
  encPinA = pinA;
  encPinB = pinB;
  pinBtn = pinBtn_;

  // Encoder pins
  pinMode(encPinA, INPUT_PULLUP);
  pinMode(encPinB, INPUT_PULLUP);

  // Init prev state
  prevAB = readAB_portable();
  isrEdges = 0;
  lastEdgeUs = micros();

  // Button init
  btnInit();

  // Attach interrupts once
  if (!isrAttached) {
    attachInterrupt(digitalPinToInterrupt(encPinA), encISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encPinB), encISR, CHANGE);
    isrAttached = true;
  }

  _inited = true;
}

EncoderEvents EncoderK040::poll() {
  EncoderEvents ev;
  if (!_inited) return ev;

  // pull edges atomically
  int16_t edges;
  noInterrupts();
  edges = isrEdges;
  isrEdges = 0;
  interrupts();

  // Convert edges -> detents (1 щелчок = ENC_DETENT_EDGES переходов)
  static int16_t edgeAcc = 0;
  edgeAcc += edges;

  static uint32_t lastStepMs = 0;
  int8_t outStep = 0;

  // выдаем максимум 1 шаг за poll(), остаток сохраняем
  if (edgeAcc >= (int16_t)ENC_DETENT_EDGES) {
    if ((uint32_t)(millis() - lastStepMs) >= ENC_STEP_GUARD_MS) {
      outStep = +1;
      edgeAcc -= ENC_DETENT_EDGES;
      lastStepMs = millis();
    }
  } else if (edgeAcc <= -(int16_t)ENC_DETENT_EDGES) {
    if ((uint32_t)(millis() - lastStepMs) >= ENC_STEP_GUARD_MS) {
      outStep = -1;
      edgeAcc += ENC_DETENT_EDGES;
      lastStepMs = millis();
    }
  }

  if (ENC_INVERT_DIR) outStep = -outStep;
  ev.step = outStep;

  // Button
  btnPoll(ev.click, ev.hold);

  return ev;
}
