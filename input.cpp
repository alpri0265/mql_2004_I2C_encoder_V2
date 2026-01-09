#include "config.h"
#include "input.h"

// ========== INPUT MODE SELECTION ==========
#ifdef INPUT_MODE_BUTTONS
  #include "input_buttons.h"
  static ButtonsInput buttons;
#else
  // Default to encoder mode
  #include "encoder_k040.h"
  static EncoderK040 encoder;
#endif

void inputBegin() {
#ifdef INPUT_MODE_BUTTONS
  // 4 кнопки: UP, DOWN, OK, MENU
  buttons.begin(PIN_BTN_UP, PIN_BTN_DOWN, PIN_BTN_OK, PIN_BTN_MENU);
#else
  // Encoder pins из config.h:
  //  PIN_BTN_UP   -> ENC A
  //  PIN_BTN_DOWN -> ENC B
  //  PIN_BTN_OK   -> ENC BTN
  encoder.begin(PIN_BTN_UP, PIN_BTN_DOWN, PIN_BTN_OK);
#endif

  pinMode(PIN_START_BTN, INPUT_PULLUP);
  pinMode(PIN_POT, INPUT);
}

void inputPoll(InputEvents &ev) {
  ev = {};

#ifdef INPUT_MODE_BUTTONS
  // Кнопки
  ButtonEvents b = buttons.poll();
  ev.encStep   = b.step;
  ev.encClick  = b.okClick;
  ev.menuClick = b.menuClick;
#else
  // Энкодер
  EncoderEvents e = encoder.poll();
  ev.encStep  = e.step;
  ev.encClick = e.click;
  // Длинное нажатие энкодера = MENU/BACK
  ev.menuClick = e.hold;
#endif

  // START кнопка (как было)
  static uint8_t lastStart = HIGH;
  uint8_t nowStart = digitalRead(PIN_START_BTN);
  if (lastStart == HIGH && nowStart == LOW) ev.startClick = true;
  lastStart = nowStart;

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
