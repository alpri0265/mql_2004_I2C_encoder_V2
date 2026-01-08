#include <Arduino.h>
#include "menu.h"
#include "ui_text_en.h"
#include "ui_text_ua.h"
#include "settings.h"
#include <avr/pgmspace.h>
#include <string.h>

// Helper function to get string from PROGMEM based on language
static void menuStrFromProgmem(char* buf, size_t bufSize, const Settings &s, const char* enStr, const char* uaStr) {
  const char* selected = (s.uiLang == UILANG_UA) ? uaStr : enStr;
  strncpy_P(buf, selected, bufSize - 1);
  buf[bufSize - 1] = '\0';
}

static constexpr uint8_t ITEM_COUNT = 19; // ✅ було 18

static int32_t clampI32(int32_t v, int32_t lo, int32_t hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

void menuReset(MenuState &m) {
  m.index = 0;
  m.editing = false;
}

// 19 символів тексту + '\0' (бо out[0] = маркер)
static void pad19(char out19[20], const char* s) {
  // Convert UTF-8 to ASCII if needed
  char converted[64];
  bool hasUtf8 = false;
  for (uint8_t k = 0; k < 64 && s[k] != '\0'; k++) {
    if ((uint8_t)s[k] >= 0xC0) {
      hasUtf8 = true;
      break;
    }
  }
  
  if (hasUtf8) {
    extern void uiConvertUtf8ToAscii(char* dest, const char* src, size_t destSize);
    uiConvertUtf8ToAscii(converted, s, sizeof(converted));
    s = converted;
  } else {
    strncpy(converted, s, sizeof(converted) - 1);
    converted[sizeof(converted) - 1] = '\0';
    s = converted;
  }
  
  uint8_t i = 0;
  for (; i < 19 && s[i] != '\0'; i++) out19[i] = s[i];
  for (; i < 19; i++) out19[i] = ' ';
  out19[19] = '\0';
}

static void makeItemLine(char out[21], uint8_t idx, char lead, const Settings &S) {
  out[0] = lead;

  char tmp[40];

  switch (idx) {
    case 0: {
      char matLabelBuf[32], steelBuf[16], alumBuf[16];
      menuStrFromProgmem(matLabelBuf, sizeof(matLabelBuf), S, UI_STR_MENU_MATERIAL_EN, UI_STR_MENU_MATERIAL_UA);
      menuStrFromProgmem(steelBuf, sizeof(steelBuf), S, UI_STR_STEEL_EN, UI_STR_STEEL_UA);
      menuStrFromProgmem(alumBuf, sizeof(alumBuf), S, UI_STR_ALUMINUM_EN, UI_STR_ALUMINUM_UA);
      snprintf(tmp, sizeof(tmp), "%s %s",
               matLabelBuf,
               (S.material == MAT_STEEL) ? steelBuf : alumBuf);
      break;
    }

    case 1: {
      char cutterLabelBuf[32], mmUnitBuf[8];
      menuStrFromProgmem(cutterLabelBuf, sizeof(cutterLabelBuf), S, UI_STR_MENU_CUTTER_EN, UI_STR_MENU_CUTTER_UA);
      menuStrFromProgmem(mmUnitBuf, sizeof(mmUnitBuf), S, UI_STR_MM_EN, UI_STR_MM_UA);
      snprintf(tmp, sizeof(tmp), "%s %u%s", cutterLabelBuf, (unsigned)S.cutter_mm, mmUnitBuf);
      break;
    }

    case 2: {
      char modeLabelBuf[32], contBuf[16], pulseBuf[16];
      menuStrFromProgmem(modeLabelBuf, sizeof(modeLabelBuf), S, UI_STR_MENU_MODE_EN, UI_STR_MENU_MODE_UA);
      menuStrFromProgmem(contBuf, sizeof(contBuf), S, UI_STR_CONT_EN, UI_STR_CONT_UA);
      menuStrFromProgmem(pulseBuf, sizeof(pulseBuf), S, UI_STR_PULSE_EN, UI_STR_PULSE_UA);
      snprintf(tmp, sizeof(tmp), "%s %s", modeLabelBuf, (S.mode == MODE_CONT) ? contBuf : pulseBuf);
      break;
    }

    case 3: {
      char pulseOnLabelBuf[32], msUnitBuf[8];
      menuStrFromProgmem(pulseOnLabelBuf, sizeof(pulseOnLabelBuf), S, UI_STR_MENU_PULSE_ON_EN, UI_STR_MENU_PULSE_ON_UA);
      menuStrFromProgmem(msUnitBuf, sizeof(msUnitBuf), S, UI_STR_MS_EN, UI_STR_MS_UA);
      snprintf(tmp, sizeof(tmp), "%s %u%s", pulseOnLabelBuf, (unsigned)S.pulse_on_ms, msUnitBuf);
      break;
    }

    case 4: {
      char pulseOffLabelBuf[32], msUnitBuf[8];
      menuStrFromProgmem(pulseOffLabelBuf, sizeof(pulseOffLabelBuf), S, UI_STR_MENU_PULSE_OFF_EN, UI_STR_MENU_PULSE_OFF_UA);
      menuStrFromProgmem(msUnitBuf, sizeof(msUnitBuf), S, UI_STR_MS_EN, UI_STR_MS_UA);
      snprintf(tmp, sizeof(tmp), "%s %u%s", pulseOffLabelBuf, (unsigned)S.pulse_off_ms, msUnitBuf);
      break;
    }

    case 5: {
      uint16_t v = S.kmin_x100;
      char kminLabelBuf[32];
      menuStrFromProgmem(kminLabelBuf, sizeof(kminLabelBuf), S, UI_STR_MENU_KMIN_EN, UI_STR_MENU_KMIN_UA);
      snprintf(tmp, sizeof(tmp), "%s %u.%02u",
               kminLabelBuf,
               (unsigned)(v / 100), (unsigned)(v % 100));
    } break;

    case 6: {
      uint16_t v = S.kmax_x100;
      char kmaxLabelBuf[32];
      menuStrFromProgmem(kmaxLabelBuf, sizeof(kmaxLabelBuf), S, UI_STR_MENU_KMAX_EN, UI_STR_MENU_KMAX_UA);
      snprintf(tmp, sizeof(tmp), "%s %u.%02u",
               kmaxLabelBuf,
               (unsigned)(v / 100), (unsigned)(v % 100));
    } break;

    case 7: {
      uint16_t v = S.al_factor_x100;
      char alFactorLabelBuf[32];
      menuStrFromProgmem(alFactorLabelBuf, sizeof(alFactorLabelBuf), S, UI_STR_MENU_ALFACTOR_EN, UI_STR_MENU_ALFACTOR_UA);
      snprintf(tmp, sizeof(tmp), "%s %u.%02u",
               alFactorLabelBuf,
               (unsigned)(v / 100), (unsigned)(v % 100));
    } break;

    case 8: {
      char potAvgLabelBuf[32];
      menuStrFromProgmem(potAvgLabelBuf, sizeof(potAvgLabelBuf), S, UI_STR_MENU_POT_AVG_EN, UI_STR_MENU_POT_AVG_UA);
      snprintf(tmp, sizeof(tmp), "%s %u", potAvgLabelBuf, (unsigned)S.pot_avg_N);
      break;
    }

    case 9: {
      uint16_t v = S.pot_hyst_x100;
      char potHystLabelBuf[32];
      menuStrFromProgmem(potHystLabelBuf, sizeof(potHystLabelBuf), S, UI_STR_MENU_POT_HYST_EN, UI_STR_MENU_POT_HYST_UA);
      snprintf(tmp, sizeof(tmp), "%s %u.%02u",
               potHystLabelBuf,
               (unsigned)(v / 100), (unsigned)(v % 100));
    } break;

    case 10: {
      char pumpGainLabelBuf[32];
      menuStrFromProgmem(pumpGainLabelBuf, sizeof(pumpGainLabelBuf), S, UI_STR_MENU_PUMPGAIN_EN, UI_STR_MENU_PUMPGAIN_UA);
      snprintf(tmp, sizeof(tmp), "%s %lu", pumpGainLabelBuf, (unsigned long)S.pump_gain_steps_per_u_min);
      break;
    }

    case 11: {
      char cal60LabelBuf[32];
      menuStrFromProgmem(cal60LabelBuf, sizeof(cal60LabelBuf), S, UI_STR_MENU_CAL_60_EN, UI_STR_MENU_CAL_60_UA);
      snprintf(tmp, sizeof(tmp), "%s", cal60LabelBuf);
      break;
    }

    case 12: {
      char cal120LabelBuf[32];
      menuStrFromProgmem(cal120LabelBuf, sizeof(cal120LabelBuf), S, UI_STR_MENU_CAL_120_EN, UI_STR_MENU_CAL_120_UA);
      snprintf(tmp, sizeof(tmp), "%s", cal120LabelBuf);
      break;
    }

    case 13: {
      char calMlULabelBuf[32];
      menuStrFromProgmem(calMlULabelBuf, sizeof(calMlULabelBuf), S, UI_STR_MENU_CAL_MLU_EN, UI_STR_MENU_CAL_MLU_UA);
      if (!S.calibrated) {
        char noneBuf[16];
        menuStrFromProgmem(noneBuf, sizeof(noneBuf), S, UI_STR_MENU_CAL_NONE_EN, UI_STR_MENU_CAL_NONE_UA);
        snprintf(tmp, sizeof(tmp), "%s %s", calMlULabelBuf, noneBuf);
      } else {
        uint32_t x = S.ml_per_u_x1000;
        uint32_t w = x / 1000;
        uint32_t f = (x % 1000) / 10; // 2 decimals
        snprintf(tmp, sizeof(tmp), "%s %lu.%02lu",
                 calMlULabelBuf,
                 (unsigned long)w, (unsigned long)f);
      }
      break;
    }

    case 14: {
      char clearCalLabelBuf[32];
      menuStrFromProgmem(clearCalLabelBuf, sizeof(clearCalLabelBuf), S, UI_STR_MENU_CLEAR_CAL_EN, UI_STR_MENU_CLEAR_CAL_UA);
      snprintf(tmp, sizeof(tmp), "%s", clearCalLabelBuf);
      break;
    }

    case 15: {
      char saveLabelBuf[32];
      menuStrFromProgmem(saveLabelBuf, sizeof(saveLabelBuf), S, UI_STR_MENU_SAVE_EN, UI_STR_MENU_SAVE_UA);
      snprintf(tmp, sizeof(tmp), "%s", saveLabelBuf);
      break;
    }

    case 16: {
      char defaultsLabelBuf[32];
      menuStrFromProgmem(defaultsLabelBuf, sizeof(defaultsLabelBuf), S, UI_STR_MENU_DEFAULTS_EN, UI_STR_MENU_DEFAULTS_UA);
      snprintf(tmp, sizeof(tmp), "%s", defaultsLabelBuf);
      break;
    }

    case 17: {
      char langLabelBuf[32], langEnBuf[16], langUaBuf[16];
      menuStrFromProgmem(langLabelBuf, sizeof(langLabelBuf), S, UI_STR_MENU_LANGUAGE_EN, UI_STR_MENU_LANGUAGE_UA);
      menuStrFromProgmem(langEnBuf, sizeof(langEnBuf), S, UI_STR_MENU_LANG_EN_EN, UI_STR_MENU_LANG_EN_UA);
      menuStrFromProgmem(langUaBuf, sizeof(langUaBuf), S, UI_STR_MENU_LANG_UA_EN, UI_STR_MENU_LANG_UA_UA);
      snprintf(tmp, sizeof(tmp), "%s %s", langLabelBuf, (S.uiLang == UILANG_UA) ? langUaBuf : langEnBuf);
      break;
    }

    case 18: {
      char lcdTestLabelBuf[32];
      menuStrFromProgmem(lcdTestLabelBuf, sizeof(lcdTestLabelBuf), S, UI_STR_MENU_LCD_TEST_EN, UI_STR_MENU_LCD_TEST_UA);
      snprintf(tmp, sizeof(tmp), "%s", lcdTestLabelBuf);
      // After LCD Test, you can use encoder to navigate through character ranges
      // Look at what characters appear and note their positions
      break;
    }

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

    case 17:
      S.uiLang = (S.uiLang == UILANG_UA) ? UILANG_EN : UILANG_UA;
      return MENU_ACT_SAVE;

    default:
      return MENU_ACT_NONE;
  }
}

MenuAction menuOnClick(MenuState &m, Settings &S) {
  if (!m.editing) {
    // "Action" items (no edit mode)
    if (m.index == 11) return MENU_ACT_CAL_START_60;
    if (m.index == 12) return MENU_ACT_CAL_START_120;
    if (m.index == 14) { m.editing = true; return MENU_ACT_NONE; }
    if (m.index == 15) return MENU_ACT_SAVE;
    if (m.index == 16) return MENU_ACT_DEFAULTS;
    if (m.index == 18) return MENU_ACT_LCD_TEST; // ✅ NEW

    // Read-only info item
    if (m.index == 13) return MENU_ACT_NONE;

    // Enter edit mode for editable items (including Language)
    m.editing = true;
    return MENU_ACT_NONE;
  } else {
    // Exit edit mode.
    m.editing = false;
    if (m.index == 14) return MENU_ACT_CAL_CLEAR; // confirm clear calibration
    if (m.index == 17) return MENU_ACT_SAVE;      // persist Language
    return MENU_ACT_NONE;
  }
}

void menuRender3(const MenuState &m, const Settings &S, char line1[21], char line2[21], char line3[21]) {
  // ✅ СТОРІНКИ по 3 пункти, щоб стрілка рухалась по рядках
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
