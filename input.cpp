#include "config.h"
#include "input.h"
#include "encoder_k040.h"

static EncoderK040 encoder;

void inputBegin() {
  // Encoder pins (см. config.h): S1->D2, S2->D12, BTN->A3
  encoder.begin(2, 12, PIN_BTN_OK);

  pinMode(PIN_START_BTN, INPUT_PULLUP);
  pinMode(PIN_POT, INPUT);
}

void inputPoll(InputEvents &ev) {
  ev = {};

  // Энкодер (EncButton v3 внутри encoder_k040.*)
  EncoderEvents e = encoder.poll();
  ev.encStep  = e.step;     // +1/-1 на один "щелчок"
  ev.encClick = e.click;    // OK
  ev.menuClick = e.hold;    // MENU/BACK (долгое нажатие)

  // encLong не используем отдельно (оставлено для совместимости)
  ev.encLong = false;

  // Потенциометр (скользящее среднее)
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
