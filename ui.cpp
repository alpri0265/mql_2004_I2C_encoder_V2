#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "ui.h"

static LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 20, 4);

static char _uiLast[4][21];
static bool _uiLastValid = false;

static void _uiSetLastBlank() {
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t i = 0; i < 20; i++) _uiLast[r][i] = ' ';
    _uiLast[r][20] = '\0';
  }
  _uiLastValid = true;
}

// ---------- UTF-8 -> PSEUDO-CYR (ASCII) ----------
// Повертає 0 якщо не вдалося прочитати символ
static uint32_t utf8ReadCodepoint(const char* s, uint8_t &adv) {
  adv = 0;
  uint8_t c0 = (uint8_t)s[0];
  if (!c0) return 0;

  if (c0 < 0x80) { adv = 1; return c0; }

  // 2 bytes
  if ((c0 & 0xE0) == 0xC0) {
    uint8_t c1 = (uint8_t)s[1];
    if (!c1) return 0;
    adv = 2;
    return ((uint32_t)(c0 & 0x1F) << 6) | (uint32_t)(c1 & 0x3F);
  }

  // 3 bytes
  if ((c0 & 0xF0) == 0xE0) {
    uint8_t c1 = (uint8_t)s[1], c2 = (uint8_t)s[2];
    if (!c1 || !c2) return 0;
    adv = 3;
    return ((uint32_t)(c0 & 0x0F) << 12) | ((uint32_t)(c1 & 0x3F) << 6) | (uint32_t)(c2 & 0x3F);
  }

  // 4 bytes (не треба)
  uint8_t c1 = (uint8_t)s[1], c2 = (uint8_t)s[2], c3 = (uint8_t)s[3];
  if (!c1 || !c2 || !c3) return 0;
  adv = 4;
  return 0;
}

static uint8_t pseudoAppend(char* out, uint8_t pos, uint8_t maxLen, const char* add) {
  while (*add && pos < maxLen) out[pos++] = *add++;
  return pos;
}

// Мапа кирилиці -> псевдокирилиця (ASCII, 1-2 символи)
static uint8_t pseudoAppendCp(char* out, uint8_t pos, uint8_t maxLen, uint32_t cp) {
  // ASCII як є
  if (cp < 0x80) {
    if (pos < maxLen) out[pos++] = (char)cp;
    return pos;
  }

  // Ukrainian/Russian Cyrillic (основні)
  // Візуально схожі: АВЕСНКМОРТХУ
  // Решта — заміни на читабельні ASCII
  const char* rep = "?";

  switch (cp) {
    // А а
    case 0x0410: case 0x0430: rep = "A"; break;
    // Б б
    case 0x0411: case 0x0431: rep = "6"; break;
    // В в
    case 0x0412: case 0x0432: rep = "B"; break;
    // Г г, Ґ ґ
    case 0x0413: case 0x0433: rep = "G"; break;
    case 0x0490: case 0x0491: rep = "G"; break;
    // Д д
    case 0x0414: case 0x0434: rep = "D"; break;
    // Е е, Є є
    case 0x0415: case 0x0435: rep = "E"; break;
    case 0x0404: case 0x0454: rep = "E"; break;
    // Ж ж
    case 0x0416: case 0x0436: rep = "X"; break;
    // З з
    case 0x0417: case 0x0437: rep = "3"; break;
    // И и, І і, Ї ї
    case 0x0418: case 0x0438: rep = "N"; break;
    case 0x0406: case 0x0456: rep = "I"; break;
    case 0x0407: case 0x0457: rep = "I"; break;
    // Й й
    case 0x0419: case 0x0439: rep = "N"; break;
    // К к
    case 0x041A: case 0x043A: rep = "K"; break;
    // Л л
    case 0x041B: case 0x043B: rep = "L"; break;
    // М м
    case 0x041C: case 0x043C: rep = "M"; break;
    // Н н
    case 0x041D: case 0x043D: rep = "H"; break;
    // О о
    case 0x041E: case 0x043E: rep = "O"; break;
    // П п
    case 0x041F: case 0x043F: rep = "P"; break;
    // Р р
    case 0x0420: case 0x0440: rep = "P"; break;
    // С с
    case 0x0421: case 0x0441: rep = "C"; break;
    // Т т
    case 0x0422: case 0x0442: rep = "T"; break;
    // У у
    case 0x0423: case 0x0443: rep = "Y"; break;
    // Ф ф
    case 0x0424: case 0x0444: rep = "F"; break;
    // Х х
    case 0x0425: case 0x0445: rep = "X"; break;
    // Ц ц
    case 0x0426: case 0x0446: rep = "U"; break;
    // Ч ч
    case 0x0427: case 0x0447: rep = "4"; break;
    // Ш ш, Щ щ
    case 0x0428: case 0x0448: rep = "W"; break;
    case 0x0429: case 0x0449: rep = "W"; break;
    // Ь ь
    case 0x042C: case 0x044C: rep = "b"; break;
    // Ю ю
    case 0x042E: case 0x044E: rep = "IO"; break;
    // Я я
    case 0x042F: case 0x044F: rep = "9"; break;
    default: rep = "?"; break;
  }

  return pseudoAppend(out, pos, maxLen, rep);
}

static void pseudoPad20(char out[21], const char* utf8) {
  // Робимо рівно 20 колонок (ASCII після конверта)
  for (uint8_t i = 0; i < 20; i++) out[i] = ' ';
  out[20] = '\0';

  uint8_t pos = 0;
  for (uint16_t si = 0; utf8[si] && pos < 20; ) {
    uint8_t adv = 0;
    uint32_t cp = utf8ReadCodepoint(&utf8[si], adv);
    if (!cp || adv == 0) break;
    si += adv;
    pos = pseudoAppendCp(out, pos, 20, cp);
  }
}

static void lcdRow(uint8_t row, const char line[21]) {
  if (!_uiLastValid) _uiSetLastBlank();
  if (memcmp(_uiLast[row], line, 20) == 0) return;

  lcd.setCursor(0, row);
  for (uint8_t i = 0; i < 20; i++) lcd.write(line[i]);
  memcpy(_uiLast[row], line, 21);
}

// ---------- PUBLIC UI ----------
void uiBegin() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  _uiLastValid = false;
  _uiSetLastBlank();
}

void uiClear() {
  lcd.clear();
  _uiLastValid = false;
  _uiSetLastBlank();
}

static const char* matUA(const Settings &S) {
  return (S.material == MAT_STEEL) ? "Сталь" : "Алюміній";
}
static const char* modeUA(const Settings &S) {
  return (S.mode == MODE_CONT) ? "Безперерв" : "Імпульс";
}

void uiDrawReady(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pseudoPad20(l0, "ГОТОВО");
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Мат: %s  O:%umm", matUA(S), (unsigned)S.cutter_mm);
    pseudoPad20(l1, tmp);
  }
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Режим: %s", modeUA(S));
    pseudoPad20(l2, tmp);
  }
  pseudoPad20(l3, "OK: Меню   START: Пуск");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawWizMaterial(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];
  pseudoPad20(l0, "МАЙСТЕР: МАТЕРІАЛ");
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "> %s", matUA(S));
    pseudoPad20(l1, tmp);
  }
  pseudoPad20(l2, "Крути: вибір");
  pseudoPad20(l3, "OK: Далі  START: Вихід");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawWizDiameter(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];
  pseudoPad20(l0, "МАЙСТЕР: ДІАМЕТР");
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "> %u мм", (unsigned)S.cutter_mm);
    pseudoPad20(l1, tmp);
  }
  pseudoPad20(l2, "Крути: вибір");
  pseudoPad20(l3, "OK: Далі  START: Вихід");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawWizRecommend(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100, int32_t potMin_u_x100, int32_t potMax_u_x100) {
  (void)potMin_u_x100; (void)potMax_u_x100;
  char l0[21], l1[21], l2[21], l3[21];

  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Реком: %ld.%02ld", (long)(rec_u_x100/100), (long)(abs(rec_u_x100)%100));
    pseudoPad20(l0, tmp);
  }
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Вст:   %ld.%02ld", (long)(set_u_x100/100), (long)(abs(set_u_x100)%100));
    pseudoPad20(l1, tmp);
  }
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Режим: %s", modeUA(S));
    pseudoPad20(l2, tmp);
  }
  pseudoPad20(l3, "START: Пуск");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawRun(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100, bool running) {
  char l0[21], l1[21], l2[21], l3[21];

  pseudoPad20(l0, running ? "ПРАЦЮЄ" : "СТОП");
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Реком:%ld.%02ld", (long)(rec_u_x100/100), (long)(abs(rec_u_x100)%100));
    pseudoPad20(l1, tmp);
  }
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Подача:%ld.%02ld", (long)(set_u_x100/100), (long)(abs(set_u_x100)%100));
    pseudoPad20(l2, tmp);
  }
  pseudoPad20(l3, "HOLD: Меню  START:Стоп");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawMenu(bool editing, const char line1[21], const char line2[21], const char line3[21]) {
  char l0[21];
  pseudoPad20(l0, editing ? "МЕНЮ (РЕДАГ)" : "МЕНЮ");
  lcdRow(0, l0);
  lcdRow(1, line1);
  lcdRow(2, line2);
  lcdRow(3, line3);
}

void uiDrawCalRun(uint16_t totalSec, uint16_t secondsLeft) {
  char l0[21], l1[21], l2[21], l3[21];
  pseudoPad20(l0, "КАЛІБРУВАННЯ");
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Трив: %us", (unsigned)totalSec);
    pseudoPad20(l1, tmp);
  }
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Зал:  %us", (unsigned)secondsLeft);
    pseudoPad20(l2, tmp);
  }
  pseudoPad20(l3, "MENU: назад");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawCalInputDigits(int32_t ml_x100, uint8_t digitIdx) {
  char l0[21], l1[21], l2[21], l3[21];

  pseudoPad20(l0, "ВВІД МЛ (60с)");
  {
    int32_t w = ml_x100 / 100;
    int32_t f = abs(ml_x100) % 100;
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Знач: %ld.%02ld", (long)w, (long)f);
    pseudoPad20(l1, tmp);
  }
  {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "Розряд: %u", (unsigned)(digitIdx + 1));
    pseudoPad20(l2, tmp);
  }
  pseudoPad20(l3, "OK: далі  MENU:наз");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}
