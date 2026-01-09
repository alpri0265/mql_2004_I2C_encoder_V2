#include <Arduino.h>
#include "config.h"
#include "pump.h"

// ============================================================
// Portable pump driver for Arduino Nano R4 (ARM) and AVR
// Uses FspTimer on Renesas, Timer1 on AVR
// ============================================================

static volatile bool stepEnable = false;
static volatile uint32_t stepIntervalUs = 10000; // микросекунды между шагами

// ==================== PLATFORM-SPECIFIC CODE ====================

#if defined(ARDUINO_ARCH_RENESAS_UNO) || defined(ARDUINO_ARCH_RENESAS)
// -------------------- Arduino Nano R4 (Renesas RA4M1) --------------------
#include "FspTimer.h"

static FspTimer pumpTimer;
static bool timerInitialized = false;

// Callback для таймера
void pumpTimerCallback(timer_callback_args_t __attribute((unused)) *p_args) {
  if (!stepEnable) return;
  
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(4);
  digitalWrite(PIN_STEP, LOW);
}

static void timerInit() {
  uint8_t timerType = GPT_TIMER;
  int8_t timerNum = FspTimer::get_available_timer(timerType);
  
  if (timerNum < 0) {
    timerType = AGT_TIMER;
    timerNum = FspTimer::get_available_timer(timerType);
  }
  
  if (timerNum >= 0) {
    pumpTimer.begin(TIMER_MODE_PERIODIC, timerType, timerNum, 100.0f, 0.0f, pumpTimerCallback);
    pumpTimer.setup_overflow_irq();
    pumpTimer.open();
    timerInitialized = true;
  }
}

static void timerSetFrequency(uint16_t hz) {
  if (!timerInitialized) return;
  if (hz < 1) hz = 1;
  if (hz > 2000) hz = 2000;
  
  pumpTimer.stop();
  pumpTimer.set_frequency((float)hz);
}

static void timerStart() {
  if (!timerInitialized) return;
  pumpTimer.start();
}

static void timerStop() {
  if (!timerInitialized) return;
  pumpTimer.stop();
}

#else
// -------------------- AVR (Arduino Uno/Nano classic) --------------------
#include <avr/io.h>
#include <avr/interrupt.h>

static void timerInit() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;

  // CTC mode
  TCCR1B |= (1 << WGM12);

  // Start disabled
  TIMSK1 &= ~(1 << OCIE1A);

  // Default OCR
  OCR1A = 2000;

  sei();
}

static void setPrescalerBits(uint16_t presc) {
  TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));

  switch (presc) {
    case 1:    TCCR1B |= (1 << CS10); break;
    case 8:    TCCR1B |= (1 << CS11); break;
    case 64:   TCCR1B |= (1 << CS11) | (1 << CS10); break;
    case 256:  TCCR1B |= (1 << CS12); break;
    case 1024: TCCR1B |= (1 << CS12) | (1 << CS10); break;
    default:   TCCR1B |= (1 << CS11); break;
  }
}

static void timerSetFrequency(uint16_t hz) {
  if (hz < 1) hz = 1;
  if (hz > 2000) hz = 2000;

  const uint16_t prescList[] = {1, 8, 64, 256, 1024};
  uint16_t bestPresc = 1024;
  uint32_t bestOcr = 65535;

  for (uint8_t i = 0; i < sizeof(prescList) / sizeof(prescList[0]); i++) {
    uint16_t p = prescList[i];
    uint32_t ocr = (F_CPU / (uint32_t)p / (uint32_t)hz) - 1UL;
    if (ocr <= 65535UL) {
      bestPresc = p;
      bestOcr = ocr;
      break;
    }
  }

  uint8_t sreg = SREG;
  cli();
  setPrescalerBits(bestPresc);
  OCR1A = (uint16_t)bestOcr;
  SREG = sreg;
}

static void timerStart() {
  TIMSK1 |= (1 << OCIE1A);
}

static void timerStop() {
  TIMSK1 &= ~(1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
  if (!stepEnable) return;

  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(4);
  digitalWrite(PIN_STEP, LOW);
}

#endif

// ==================== COMMON API ====================

void pumpBegin() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);

  digitalWrite(PIN_STEP, LOW);
  digitalWrite(PIN_DIR, HIGH);   // направление любое
  digitalWrite(PIN_ENA, HIGH);   // ENA polarity inverted: HIGH = disabled

  timerInit();
}

void pumpSetEnable(bool en) {
  stepEnable = en;

  if (en) {
    timerStart();
  } else {
    timerStop();
  }

  digitalWrite(PIN_ENA, en ? LOW : HIGH);  // ENA polarity inverted
}

void pumpStartSteps(uint32_t stepsPerSec) {
  if (stepsPerSec == 0) {
    pumpStop();
    return;
  }
  if (stepsPerSec > 2000) stepsPerSec = 2000;
  timerSetFrequency((uint16_t)stepsPerSec);
  pumpSetEnable(true);
}

void pumpStop() {
  stepEnable = false;
  timerStop();
  digitalWrite(PIN_ENA, HIGH);  // disable (inverted)
}

void pumpRunCont(int32_t flow_x100, uint32_t pumpGain) {
  if (flow_x100 <= 0 || pumpGain == 0) {
    pumpStop();
    return;
  }

  // steps/min = flow * gain
  uint64_t stepsPerMin = ((uint64_t)flow_x100 * (uint64_t)pumpGain) / 100ULL;
  if (stepsPerMin == 0) {
    pumpStop();
    return;
  }

  // steps/sec (округление вверх)
  uint32_t hz = (uint32_t)((stepsPerMin + 59ULL) / 60ULL);
  if (hz < 1) hz = 1;
  if (hz > 2000) hz = 2000;

  timerSetFrequency((uint16_t)hz);
  pumpSetEnable(true);
}

void pumpRunPulse(bool &phaseOn,
                  uint32_t &phaseStartMs,
                  const Settings &S,
                  int32_t flow_x100) {
  (void)phaseOn;
  (void)phaseStartMs;
  (void)S;
  (void)flow_x100;
}
