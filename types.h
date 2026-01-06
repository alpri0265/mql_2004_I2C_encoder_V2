#pragma once
#include <Arduino.h>

// Avoid macro name collisions if config.h defines UI_LANG_EN/UI_LANG_UA
#ifdef UI_LANG_EN
  #undef UI_LANG_EN
#endif
#ifdef UI_LANG_UA
  #undef UI_LANG_UA
#endif
#ifdef UI_LANG
  #undef UI_LANG
#endif

// UI language stored in EEPROM
enum UiLang : uint8_t {
  UILANG_EN = 0,
  UILANG_UA = 1
};


// Перечисления
enum Material : uint8_t {
  MAT_STEEL,
  MAT_ALUMINUM
};

enum Mode : uint8_t {
  MODE_CONT,
  MODE_PULSE
};

enum AppState : uint8_t {
  ST_READY,
  ST_RUN,
  ST_MENU,
  ST_WIZ_MAT,
  ST_WIZ_DIA,
  ST_WIZ_REC,
  ST_CAL_RUN,
  ST_CAL_INPUT
};

// Структура настроек
struct Settings {
  uint32_t magic;


  UiLang   uiLang;
  Material material;
  uint8_t  cutter_mm;

  Mode     mode;
  uint16_t pulse_on_ms;
  uint16_t pulse_off_ms;

  uint16_t kmin_x100;
  uint16_t kmax_x100;
  uint16_t al_factor_x100;

  uint8_t  pot_avg_N;
  uint8_t  pot_hyst_x100;

  uint32_t pump_gain_steps_per_u_min;
  uint32_t steps_per_rev;

  bool     calibrated;
  uint32_t ml_per_u_x1000;

  int32_t  last_rec_x100;
};

// Структура событий энкодера
struct EncoderEvents {
  int8_t  step  = 0;   // +1/-1 на один "щелчок"
  bool    click = false; // короткое нажатие
  bool    hold  = false; // длительное нажатие
};

// Глобальная переменная настроек
extern Settings S;