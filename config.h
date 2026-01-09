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

// ===== INPUT MODE =====
// Uncomment ONE of the following:
#define INPUT_MODE_BUTTONS  // Use 4 separate buttons (UP, DOWN, OK, MENU)
// #define INPUT_MODE_ENCODER  // Use KY-040 rotary encoder

// ===== BUTTON PINS (for INPUT_MODE_BUTTONS) =====
// All buttons connect to GND when pressed (INPUT_PULLUP)
constexpr uint8_t PIN_BTN_UP   = 2;       // UP button -> GND
constexpr uint8_t PIN_BTN_DOWN = 3;       // DOWN button -> GND
constexpr uint8_t PIN_BTN_OK   = A3;      // OK/ENTER button -> GND
constexpr uint8_t PIN_BTN_MENU = 4;       // MENU/BACK button -> GND

// ===== BUTTON TIMING =====
constexpr uint8_t BTN_DEBOUNCE_MS = 25;   // Debounce time, ms
constexpr uint16_t BTN_REPEAT_DELAY_MS = 400;  // Delay before auto-repeat starts
constexpr uint16_t BTN_REPEAT_RATE_MS = 100;   // Auto-repeat interval

// ===== ENCODER CONFIG (for INPUT_MODE_ENCODER) =====
// Декодер "классический" по таблице переходов (не зависит от EB_STEP*).
// Настройка под ваш механический энкодер (KY-040): обычно 4 перехода на один "щелчок".
// Если у вас энкодер другого типа и 1 щелчок = 2 перехода, поставьте 2.
constexpr uint8_t ENC_DETENT_EDGES = 4;  // 4 = типично для KY-040, 2 = для некоторых EC11

// Инверсия направления энкодера (0/1)
constexpr bool ENC_INVERT_DIR = true;

// Микро-защита от дребезга: слишком быстрые изменения A/B игнорируются
// (для KY-040 помогает убрать "фантомные" шаги)
constexpr uint16_t ENC_MIN_EDGE_US = 300; // 200..800 мкс, начните с 300

// Минимальная пауза между ВЫДАННЫМИ шагами (не между переходами)
constexpr uint8_t ENC_STEP_GUARD_MS = 2; // 0..5 мс

// Кнопка энкодера (OK)
constexpr uint8_t ENC_BTN_DEBOUNCE_MS = 25; // 15..40 мс
constexpr uint16_t ENC_BTN_LONG_MS = 600;   // длительное нажатие, мс

// ===== Тайминги =====
constexpr uint16_t INPUT_POLL_MS = 5;
constexpr uint16_t UI_REFRESH_MS = 200;
// =====================
// UI language selection
// =====================

// Language IDs
#define UI_LANG_EN  0
#define UI_LANG_UA  1

// Active UI language
// ⚠️ В main має лишатися UI_LANG_EN
// ⚠️ В гілці feature/ua-ui-cyrillic можна поставити UI_LANG_UA
#define UI_LANG UI_LANG_EN

