#pragma once
#include <Arduino.h>

// ===== LCD 2004 I2C =====
constexpr uint8_t LCD_I2C_ADDR = 0x27;   // поменяй на 0x3F если у тебя так

// ===== POT =====
constexpr uint8_t PIN_POT = A0;

// ===== START button + LED =====
constexpr uint8_t PIN_START_BTN = A1;    // кнопка START/STOP (NO) -> GND
constexpr uint8_t PIN_START_LED = A2;    // LED кнопки START

// ===== DM556 =====
constexpr uint8_t PIN_STEP = 12;          // PUL+
constexpr uint8_t PIN_DIR  = 10;         // DIR+
constexpr uint8_t PIN_ENA  = 11;         // ENA+

// ===== INPUT (KY-040 encoder via EncButton v3) =====
// Encoder pins (KY-040): S1->D2, S2->D12, BTN->A3
constexpr uint8_t PIN_BTN_UP   = 2;       // ENC A (S1)  (to GND via encoder)
constexpr uint8_t PIN_BTN_DOWN = 3;      // ENC B (S2)  (to GND via encoder)
constexpr uint8_t PIN_BTN_OK   = A3;      // ENC BTN     (to GND via encoder)

// ===================== ENCODER CONFIG =====================
// Декодер "классический" по таблице переходов (не зависит от EB_STEP*).
// Настройка под ваш механический энкодер (KY-040): обычно 4 перехода на один "щелчок".
// Если у вас энкодер другого типа и 1 щелчок = 2 перехода, поставьте 2.
constexpr uint8_t ENC_DETENT_EDGES = 4;  // 4 = типично для KY-040, 2 = для некоторых EC11

// Инверсия направления энкодера (0/1)
constexpr bool ENC_INVERT_DIR = false;

// Микро-защита от дребезга: слишком быстрые изменения A/B игнорируются
// (для KY-040 помогает убрать "фантомные" шаги)
constexpr uint16_t ENC_MIN_EDGE_US = 300; // 200..800 мкс, начните с 300

// Минимальная пауза между ВЫДАННЫМИ шагами (не между переходами)
constexpr uint8_t ENC_STEP_GUARD_MS = 2; // 0..5 мс

// Кнопка энкодера (OK)
constexpr uint8_t ENC_BTN_DEBOUNCE_MS = 25; // 15..40 мс
constexpr uint16_t ENC_BTN_LONG_MS = 600;   // длительное нажатие, мс

// Old MENU button is no longer used (BACK/MENU = encoder hold).
// Leave pin defined but do not connect anything to it:
constexpr uint8_t PIN_BTN_MENU = 4;       // (unused)

// ===== Тайминги =====
constexpr uint16_t INPUT_POLL_MS = 5;
constexpr uint16_t UI_REFRESH_MS = 200;
