#include "config.h"
#include "input.h"
#include "encoder_k040.h"

static EncoderK040 encoder;

void inputBegin() {
  encoder.begin(2, 12, A3);
  pinMode(PIN_START_BTN, INPUT_PULLUP);
  pinMode(PIN_POT, INPUT);
}

void inputPoll(InputEvents &ev) {
  ev = {}; 
  uint32_t now = millis();

  EncoderEvents e = encoder.poll();

  // Шаги энкодера — с лёгким накоплением, чтобы избежать пропусков
  static int8_t subStep = 0;
  if (e.step != 0) {
    subStep += e.step;
    if (abs(subStep) >= 2) {  // 2 — чтобы сработка была после 1–2 импульсов, а не 10
      ev.encStep = (subStep > 0) ? 1 : -1;
      subStep = 0;
    }
  }

  // Кнопка энкодера — короткое и длинное нажатие
  static uint32_t buttonPressTime = 0;
  static bool buttonPressed = false;

  bool buttonNow = (digitalRead(PIN_BTN_OK) == LOW);

  if (buttonNow && !buttonPressed) {
      buttonPressed = true;
      buttonPressTime = now;
  } else if (!buttonNow && buttonPressed) {
      buttonPressed = false;
      uint32_t pressDuration = now - buttonPressTime;
      if (pressDuration >= 600) {  // длинное нажатие для меню
          ev.menuClick = true;
      } else if (pressDuration >= 25) {  // короткое для OK
          ev.encClick = true;
      }
  }

  ev.encLong = false;  // не используем

  // Потенциометр
  static uint16_t pAvg = 0;
  pAvg = (pAvg * 7 + analogRead(PIN_POT)) / 8;
}

uint16_t potGetAvgAdc() { 
  static uint16_t lastPot = 0;
  lastPot = (lastPot * 7 + analogRead(PIN_POT)) / 8;
  return lastPot; 
}

void potSetFilterN(uint8_t N) { 
  (void)N; 
}