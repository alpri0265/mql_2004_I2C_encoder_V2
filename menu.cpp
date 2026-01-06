#include <Arduino.h>
#include "menu.h"

static constexpr uint8_t ITEM_COUNT = 17;
static int32_t clampI32(int32_t v, int32_t lo, int32_t hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

void menuReset(MenuState &m) {
  m.index = 0;
  m.editing = false;
}

// 19 chars + '\0' (out[0] = marker)
static void pad19(char out19[20], const char* s) {
  uint8_t i = 0;
  for (; i < 19 && s[i]; i++) out19[i] = s[i];
  for (; i < 19; i++) out19[i] = ' ';
  out19[19] = '\0';
}

static void makeItemLine(char out[21], uint8_t idx, char lead, const Settings &S) {
  out[0] = lead;

  char tmp[32];

  switch (idx) {
    case 0:
      snprintf(tmp, sizeof(tmp), "Material: %s", (S.material == MAT_STEEL) ? "Stal" : "Alum");
      break;
    case 1:
      snprintf(tmp, sizeof(tmp), "Freza O: %umm", (unsigned)S.cutter_mm);
      break;
    case 2:
      snprintf(tmp, sizeof(tmp), "Rezhym: %s", (S.mode == MODE_CONT) ? "POST" : "IMP");
      break;
    case 3:
      snprintf(tmp, sizeof(tmp), "Imp ON: %ums", (unsigned)S.pulse_on_ms);
      break;
    case 4:
      snprintf(tmp, sizeof(tmp), "Imp OFF:%ums", (unsigned)S.pulse_off_ms);
      break;
    case 5: {
      uint16_t v = S.kmin_x100;
      snprintf(tmp, sizeof(tmp), "Kmin: %u.%02u", (unsigned)(v/100), (unsigned)(v%100));
    } break;
    case 6: {
      uint16_t v = S.kmax_x100;
      snprintf(tmp, sizeof(tmp), "Kmax: %u.%02u", (unsigned)(v/100), (unsigned)(v%100));
    } break;
    case 7: {
      uint16_t v = S.al_factor_x100;
      snprintf(tmp, sizeof(tmp), "Koeff Al:%u.%02u", (unsigned)(v/100), (unsigned)(v%100));
    } break;
    case 8:
      snprintf(tmp, sizeof(tmp), "POT avg N: %u", (unsigned)S.pot_avg_N);
      break;
    case 9: {
      uint16_t v = S.pot_hyst_x100;
      snprintf(tmp, sizeof(tmp), "POT hyst:%u.%02u", (unsigned)(v/100), (unsigned)(v%100));
    } break;
    case 10:
      snprintf(tmp, sizeof(tmp), "Pidsyl: %lu", (unsigned long)S.pump_gain_steps_per_u_min);
      break;
    case 11:
      snprintf(tmp, sizeof(tmp), "Kalibr 60s");
      break;
    case 12:
      snprintf(tmp, sizeof(tmp), "Kalibr 120s");
      break;
    case 13:
      if (!S.calibrated) snprintf(tmp, sizeof(tmp), "Kal ml/od: (--)");
      else {
        uint32_t x = S.ml_per_u_x1000;
        uint32_t w = x / 1000;
        uint32_t f = (x % 1000) / 10;
        snprintf(tmp, sizeof(tmp), "Kal ml/od: %lu.%02lu", (unsigned long)w, (unsigned long)f);
      }
      break;
    case 14:
      snprintf(tmp, sizeof(tmp), "Skynuty kalibr");
      break;
    case 15:
      snprintf(tmp, sizeof(tmp), "Zberegty EEPROM");
      break;
    case 16:
      snprintf(tmp, sizeof(tmp), "Za zamovch (def)");
      break;
    default:
      snprintf(tmp, sizeof(tmp), "-");
      break;
  }

  char padded[20];
  pad19(padded, tmp);
  for (uint8_t i = 0; i < 19; i++) out[1 + i] = padded[i];
  out[20] = '\0';
}

MenuAction menuOnDelta(MenuState &m, int8_t step, Settings &S) {
  if (step == 0) return MENU_ACT_NONE;

  if (!m.editing) {
    int16_t ni = (int16_t)m.index + step;
    if (ni < 0) ni = 0;
    if (ni >= ITEM_COUNT) ni = ITEM_COUNT - 1;
    m.index = (uint8_t)ni;
    return MENU_ACT_NONE;
  }

  switch (m.index) {
    case 0:
      S.material = (step > 0) ? MAT_ALUMINUM : MAT_STEEL;
      return MENU_ACT_RECOMPUTE;
    case 1:
      S.cutter_mm = (uint8_t)clampI32((int32_t)S.cutter_mm + step, 3, 50);
      return MENU_ACT_RECOMPUTE;
    case 2:
      S.mode = (S.mode == MODE_CONT) ? MODE_PULSE : MODE_CONT;
      return MENU_ACT_NONE;
    case 3:
      S.pulse_on_ms = (uint16_t)clampI32((int32_t)S.pulse_on_ms + step * 50, 100, 5000);
      return MENU_ACT_NONE;
    case 4:
      S.pulse_off_ms = (uint16_t)clampI32((int32_t)S.pulse_off_ms + step * 100, 100, 10000);
      return MENU_ACT_NONE;
    case 5:
      S.kmin_x100 = (uint16_t)clampI32((int32_t)S.kmin_x100 + step * 2, 20, 100);
      return MENU_ACT_RECOMPUTE;
    case 6:
      S.kmax_x100 = (uint16_t)clampI32((int32_t)S.kmax_x100 + step * 5, 120, 400);
      return MENU_ACT_RECOMPUTE;
    case 7:
      S.al_factor_x100 = (uint16_t)clampI32((int32_t)S.al_factor_x100 + step * 2, 100, 200);
      return MENU_ACT_RECOMPUTE;
    case 8: {
      uint8_t n = S.pot_avg_N;
      if (step > 0) { if (n == 4) n = 8; else if (n == 8) n = 16; }
      else          { if (n == 16) n = 8; else if (n == 8) n = 4; }
      S.pot_avg_N = n;
      return MENU_ACT_NONE;
    }
    case 9:
      S.pot_hyst_x100 = (uint8_t)clampI32((int32_t)S.pot_hyst_x100 + step, 0, 50);
      return MENU_ACT_NONE;
    case 10: {
      int32_t g = (int32_t)S.pump_gain_steps_per_u_min;
      g += step * 50;
      g = clampI32(g, 50, 50000);
      S.pump_gain_steps_per_u_min = (uint32_t)g;
      return MENU_ACT_NONE;
    }
    default:
      return MENU_ACT_NONE;
  }
}

MenuAction menuOnClick(MenuState &m, Settings &S) {
  (void)S;
  if (!m.editing) {
    if (m.index == 11) return MENU_ACT_CAL_START_60;
    if (m.index == 12) return MENU_ACT_CAL_START_120;
    if (m.index == 14) return MENU_ACT_CAL_CLEAR;
    if (m.index == 15) return MENU_ACT_SAVE;
    if (m.index == 16) return MENU_ACT_DEFAULTS;
    if (m.index == 13) return MENU_ACT_NONE; // read-only
    m.editing = true;
    return MENU_ACT_NONE;
  } else {
    m.editing = false;
    return MENU_ACT_NONE;
  }
}

void menuRender3(const MenuState &m, const Settings &S, char line1[21], char line2[21], char line3[21]) {
  uint8_t top = (m.index / 3) * 3;
  if (top > ITEM_COUNT - 3) top = ITEM_COUNT - 3;

  for (uint8_t row = 0; row < 3; row++) {
    uint8_t idx = top + row;
    bool sel = (m.index == idx);

    char lead = ' ';
    if (sel) lead = (m.editing ? '*' : '>');

    char* out = (row == 0) ? line1 : (row == 1) ? line2 : line3;
    makeItemLine(out, idx, lead, S);
  }
}
