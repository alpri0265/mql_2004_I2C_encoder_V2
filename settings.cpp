#include <EEPROM.h>
#include "settings.h"

Settings S;
static constexpr uint32_t SETTINGS_MAGIC = 0x4D514C31UL; // "MQL1"

void settingsLoadDefaults() {
  S.magic = SETTINGS_MAGIC;


  S.uiLang = UILANG_EN;
  S.material = MAT_STEEL;
  S.cutter_mm = 10;

  S.mode = MODE_CONT;
  S.pulse_on_ms = 500;
  S.pulse_off_ms = 2000;

  S.kmin_x100 = 50;       // 0.50x
  S.kmax_x100 = 200;      // 2.00x
  S.al_factor_x100 = 130; // Al = 1.30x Steel

  S.pot_avg_N = 8;
  S.pot_hyst_x100 = 2;    // 0.02 u/min hysteresis (in u units)

  S.pump_gain_steps_per_u_min = 1000;
  S.steps_per_rev = 3200;

  S.calibrated = false;
  S.ml_per_u_x1000 = 0;

  S.last_rec_x100 = 55;
}

void settingsLoad() {
  EEPROM.get(0, S);

  // If EEPROM is empty/old layout -> reset to defaults
  if (S.magic != SETTINGS_MAGIC) {
    settingsLoadDefaults();
    settingsSave();
    return;
  }

  bool bad = false;

  // --- validate enums / ranges (protect against old EEPROM layout corruption) ---
  if (S.uiLang != UILANG_EN && S.uiLang != UILANG_UA) bad = true;
  if (S.material > MAT_ALUMINUM) bad = true;
  if (S.mode > MODE_PULSE) bad = true;

  if (S.cutter_mm < 1 || S.cutter_mm > 60) bad = true;

  if (S.pulse_on_ms < 10 || S.pulse_on_ms > 10000) bad = true;
  if (S.pulse_off_ms < 10 || S.pulse_off_ms > 60000) bad = true;

  // kmin/kmax are factors (x100). Realistic limits:
  if (S.kmin_x100 < 10 || S.kmin_x100 > 300) bad = true;   // 0.10x .. 3.00x
  if (S.kmax_x100 < 20 || S.kmax_x100 > 600) bad = true;   // 0.20x .. 6.00x
  if (S.kmax_x100 <= S.kmin_x100) bad = true;

  if (S.al_factor_x100 < 50 || S.al_factor_x100 > 300) bad = true;

  if (S.pot_avg_N < 1 || S.pot_avg_N > 32) bad = true;
  if (S.pot_hyst_x100 > 50) bad = true;

  if (S.pump_gain_steps_per_u_min < 1 || S.pump_gain_steps_per_u_min > 2000000UL) bad = true;
  if (S.steps_per_rev < 200 || S.steps_per_rev > 50000UL) bad = true;

  if (S.calibrated) {
    if (S.ml_per_u_x1000 == 0 || S.ml_per_u_x1000 > 5000000UL) bad = true;
  }

  // last_rec is x100; protect from nonsense
  if (S.last_rec_x100 < 1 || S.last_rec_x100 > 50000) bad = true;

  if (bad) {
    // Preserve language if it looks valid, otherwise default.
    UiLang keepLang = (S.uiLang == UILANG_EN || S.uiLang == UILANG_UA) ? S.uiLang : UILANG_EN;

    settingsLoadDefaults();
    S.uiLang = keepLang;

    settingsSave();
  }
}


void settingsSave() {
  S.magic = SETTINGS_MAGIC;
  // Do NOT overwrite uiLang here (it is user-selected)
  EEPROM.put(0, S);
}

