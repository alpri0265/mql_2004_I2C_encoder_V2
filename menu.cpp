#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"

// Items count must match your original
static constexpr uint8_t ITEM_COUNT = 17;

static int32_t clampI32(int32_t v, int32_t lo, int32_t hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

// --- same UTF-8 -> pseudo-cyr helpers (local to this file) ---
static uint32_t utf8ReadCodepoint(const char* s, uint8_t &adv) {
  adv = 0;
  uint8_t c0 = (uint8_t)s[0];
  if (!c0) return 0;
  if (c0 < 0x80) { adv = 1; return c0; }
  if ((c0 & 0xE0) == 0xC0) {
    uint8_t c1 = (uint8_t)s[1];
    if (!c1) return 0;
    adv = 2;
    return ((uint32_t)(c0 & 0x1F) << 6) | (uint32_t)(c1 & 0x3F);
  }
  if ((c0 & 0xF0) == 0xE0) {
    uint8_t c1 = (uint8_t)s[1], c2 = (uint8_t)s[2];
    if (!c1 || !c2) return 0;
    adv = 3;
    return ((uint32_t)(c0 & 0x0F) << 12) | ((uint32_t)(c1 & 0x3F) << 6) | (uint32_t)(c2 & 0x3F);
  }
  uint8_t c1 = (uint8_t)s[1], c2 = (uint8_t)s[2], c3 = (uint8_t)s[3];
  if (!c1 || !c2 || !c3) return 0;
  adv = 4;
  return 0;
}

static uint8_t pseudoAppend(char* out, uint8_t pos, uint8_t maxLen, const char* add) {
  while (*add && pos < maxLen) out[pos++] = *add++;
  return pos;
}

static uint8_t pseudoAppendCp(char* out, uint8_t pos, uint8_t maxLen, uint32_t cp) {
  if (cp < 0x80) {
    if (pos < maxLen) out[pos++] = (char)cp;
    return pos;
  }

  const char* rep = "?";
  switch (cp) {
    case 0x0410: case 0x0430: rep = "A"; break;
    case 0x0411: case 0x0431: rep = "6"; break;
    case 0x0412: case 0x0432: rep = "B"; break;
    case 0x0413: case 0x0433: rep = "G"; break;
    case 0x0490: case 0x0491: rep = "G"; break;
    case 0x0414: case 0x0434: rep = "D"; break;
    case 0x0415: case 0x0435: rep = "E"; break;
    case 0x0404: case 0x0454: rep = "E"; break;
    case 0x0416: case 0x0436: rep = "X"; break;
    case 0x0417: case 0x0437: rep = "3"; break;
    case 0x0418: case 0x0438: rep = "N"; break;
    case 0x0406: case 0x0456: rep = "I"; break;
    case 0x0407: case 0x0457: rep = "I"; break;
    case 0x0419: case 0x0439: rep = "N"; break;
    case 0x041A: case 0x043A: rep = "K"; break;
    case 0x041B: case 0x043B: rep = "L"; break;
    case 0x041C: case 0x043C: rep = "M"; break;
    case 0x041D: case 0x043D: rep = "H"; break;
    case 0x041E: case 0x043E: rep = "O"; break;
    case 0x041F: case 0x043F: rep = "P"; break;
    case 0x0420: case 0x0440: rep = "P"; break;
    case 0x0421: case 0x0441: rep = "C"; break;
    case 0x0422: case 0x0442: rep = "T"; break;
    case 0x0423: case 0x0443: rep = "Y"; break;
    case 0x0424: case 0x0444: rep = "F"; break;
    case 0x0425: case 0x0445: rep = "X"; break;
    case 0x0426: case 0x0446: rep = "U"; break;
    case 0x0427: case 0x0447: rep = "4"; break;
    case 0x0428: case 0x0448: rep = "W"; break;
    case 0x0429: case 0x0449: rep = "W"; break;
    case 0x042C: case 0x044C: rep = "b"; break;
    case 0x042E: case 0x044E: rep = "IO"; break;
    case 0x042F: case 0x044F: rep = "9"; break;
    default: rep = "?"; break;
  }
  return pseudoAppend(out, pos, maxLen, rep);
}

static void pseudoLine20(char out[21], char lead, const char* utf8Text) {
  out[0] = lead;
  for (uint8_t i = 1; i < 20; i++) out[i] = ' ';
  out[20] = '\0';

  uint8_t pos = 1;
  for (uint16_t si = 0; utf8Text[si] && pos < 20; ) {
    uint8_t adv = 0;
    uint32_t cp = utf8ReadCodepoint(&utf8Text[si], adv);
    if (!cp || adv == 0) break;
    si += adv;
    pos = pseudoAppendCp(out, pos, 20, cp);
  }
}

// helpers
static const char* matUA(const Settings &S) {
  return (S.material == MAT_STEEL) ? "Сталь" : "Алюміній";
}
static const char* modeUA(const Settings &S) {
  return (S.mode == MODE_CONT) ? "Безперерв" : "Імпульс";
}

void menuReset(MenuState &m) {
  m.index = 0;
  m.editing = false;
}

static void makeItemLine(char out[21], uint8_t idx, bool selected, bool editing, const Settings &S) {
  char lead = ' ';
  if (selected) lead = editing ? '*' : '>';

  // Формуємо маленькі ASCII-частини (цифри) окремо, а текст — UTF-8 -> pseudo
  char tmp[64];

  switch (idx) {
    case 0:
      snprintf(tmp, sizeof(tmp), "Матеріал: %s", matUA(S));
      pseudoLine20(out, lead, tmp);
      break;

    case 1:
      snprintf(tmp, sizeof(tmp), "Фреза O: %umm", (unsigned)S.cutter_mm);
      pseudoLine20(out, lead, tmp);
      break;

    case 2:
      snprintf(tmp, sizeof(tmp), "Режим: %s", modeUA(S));
      pseudoLine20(out, lead, tmp);
      break;

    case 3:
      snprintf(tmp, sizeof(tmp), "Імп ON: %ums", (unsigned)S.pulse_on_ms);
      pseudoLine20(out, lead, tmp);
      break;

    case 4:
      snprintf(tmp, sizeof(tmp), "Імп OFF:%ums", (unsigned)S.pulse_off_ms);
      pseudoLine20(out, lead, tmp);
      break;

    case 5: {
      uint16_t v = S.kmin_x100;
      snprintf(tmp, sizeof(tmp), "Kmin: %u.%02u", (unsigned)(v/100), (unsigned)(v%100));
      pseudoLine20(out, lead, tmp);
    } break;

    case 6: {
      uint16_t v = S.kmax_x100;
      snprintf(tmp, sizeof(tmp), "Kmax: %u.%02u", (unsigned)(v/100), (unsigned)(v%100));
      pseudoLine20(out, lead, tmp);
    } break;

    case 7: {
      uint16_t v = S.al_factor_x100;
      snprintf(tmp, sizeof(tmp), "Al коеф: %u.%02u", (unsigned)(v/100), (unsigned)(v%100));
      pseudoLine20(out, lead, tmp);
    } break;

    case 8:
      snprintf(tmp, sizeof(tmp), "POT avg N: %u", (unsigned)S.pot_avg_N);
      pseudoLine20(out, lead, tmp);
      break;

    case 9: {
      uint16_t v = S.pot_hyst_x100;
      snprintf(tmp, sizeof(tmp), "POT гіст: %u.%02u", (unsigned)(v/100), (unsigned)(v%100));
      pseudoLine20(out, lead, tmp);
    } break;

    case 10:
      snprintf(tmp, sizeof(tmp), "Підсил: %lu", (unsigned long)S.pump_gain_steps_per_u_min);
      pseudoLine20(out, lead, tmp);
      break;

    case 11:
      pseudoLine20(out, lead, "Калібр 60с");
      break;

    case 12:
      pseudoLine20(out, lead, "Калібр 120с");
      break;

    case 13:
      if (!S.calibrated) pseudoLine20(out, lead, "Калібр мл/од: (нема)");
      else {
        uint32_t x = S.ml_per_u_x1000;
        uint32_t w = x / 1000;
        uint32_t f = (x % 1000) / 10;
        snprintf(tmp, sizeof(tmp), "Калібр мл/од:%lu.%02lu", (unsigned long)w, (unsigned long)f);
        pseudoLine20(out, lead, tmp);
      }
      break;

    case 14:
      pseudoLine20(out, lead, "Скинути калібр");
      break;

    case 15:
      pseudoLine20(out, lead, "Зберегти EEPROM");
      break;

    case 16:
      pseudoLine20(out, lead, "Заводські налашт");
      break;

    default:
      pseudoLine20(out, lead, "-");
      break;
  }
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
  // “курсор рухається по екрану”, а не “стоїть знизу”
  uint8_t top = (m.index / 3) * 3;
  if (top > ITEM_COUNT - 3) top = ITEM_COUNT - 3;

  makeItemLine(line1, top + 0, (m.index == top + 0), m.editing, S);
  makeItemLine(line2, top + 1, (m.index == top + 1), m.editing, S);
  makeItemLine(line3, top + 2, (m.index == top + 2), m.editing, S);
}
