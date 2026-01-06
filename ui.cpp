#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "ui.h"

static LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 20, 4);

// cache last drawn lines
static char last4[4][21];
static bool lastValid = false;

static void setLastBlank() {
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t i = 0; i < 20; i++) last4[r][i] = ' ';
    last4[r][20] = '\0';
  }
  lastValid = true;
}

static void pad20(char out[21], const char* s) {
  uint8_t i = 0;
  for (; i < 20 && s[i]; i++) out[i] = s[i];
  for (; i < 20; i++) out[i] = ' ';
  out[20] = '\0';
}

static void drawRow(uint8_t row, const char line[21]) {
  if (!lastValid) setLastBlank();
  if (memcmp(last4[row], line, 20) == 0) return;

  lcd.setCursor(0, row);
  for (uint8_t i = 0; i < 20; i++) lcd.write(line[i]);
  memcpy(last4[row], line, 21);
}

static void draw4(const char l0[21], const char l1[21], const char l2[21], const char l3[21]) {
  drawRow(0, l0);
  drawRow(1, l1);
  drawRow(2, l2);
  drawRow(3, l3);
}

void uiBegin() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lastValid = false;
  setLastBlank();
}

void uiClear() {
  lcd.clear();
  lastValid = false;
  setLastBlank();
}

static const char* matStr(const Settings &S) {
  return (S.material == MAT_STEEL) ? "Сталь" : "Алюм";
}
static const char* modeStr(const Settings &S) {
  return (S.mode == MODE_CONT) ? "ПОСТ" : "ІМП";
}

void uiDrawReady(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];
  pad20(l0, "ГОТОВО");
  {
    char b[32];
    snprintf(b, sizeof(b), "Мат:%s  O:%umm", matStr(S), (unsigned)S.cutter_mm);
    pad20(l1, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "Режим:%s", modeStr(S));
    pad20(l2, b);
  }
  pad20(l3, "OK:Меню  START:Пуск");
  draw4(l0, l1, l2, l3);
}

void uiDrawWizMaterial(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];
  pad20(l0, "МАЙСТЕР: МАТЕРІАЛ");
  {
    char b[32];
    snprintf(b, sizeof(b), "> %s", matStr(S));
    pad20(l1, b);
  }
  pad20(l2, "Крути: змінити");
  pad20(l3, "OK:Далі  MENU:Назад");
  draw4(l0, l1, l2, l3);
}

void uiDrawWizDiameter(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];
  pad20(l0, "МАЙСТЕР: ФРЕЗА O");
  {
    char b[32];
    snprintf(b, sizeof(b), "> %umm", (unsigned)S.cutter_mm);
    pad20(l1, b);
  }
  pad20(l2, "Крути: змінити");
  pad20(l3, "OK:Далі  MENU:Назад");
  draw4(l0, l1, l2, l3);
}

void uiDrawWizRecommend(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100,
                        int32_t potMin_u_x100, int32_t potMax_u_x100) {
  (void)S;
  char l0[21], l1[21], l2[21], l3[21];
  pad20(l0, "МАЙСТЕР: РЕКОМЕНД.");
  {
    char b[32];
    snprintf(b, sizeof(b), "Рек: %ld.%02ld од",
             (long)(rec_u_x100 / 100), (long)(abs(rec_u_x100) % 100));
    pad20(l1, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "Вст: %ld.%02ld од",
             (long)(set_u_x100 / 100), (long)(abs(set_u_x100) % 100));
    pad20(l2, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "POT:%ld..%ld",
             (long)(potMin_u_x100 / 100), (long)(potMax_u_x100 / 100));
    pad20(l3, b);
  }
  draw4(l0, l1, l2, l3);
}

void uiDrawRun(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100, bool running) {
  char l0[21], l1[21], l2[21], l3[21];
  {
    char b[32];
    snprintf(b, sizeof(b), "РОБОТА: %s", running ? "ВКЛ" : "ВИКЛ");
    pad20(l0, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "Рек:%ld.%02ld  %s",
             (long)(rec_u_x100 / 100), (long)(abs(rec_u_x100) % 100), modeStr(S));
    pad20(l1, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "Вст:%ld.%02ld  O:%u",
             (long)(set_u_x100 / 100), (long)(abs(set_u_x100) % 100), (unsigned)S.cutter_mm);
    pad20(l2, b);
  }
  pad20(l3, "START:Стоп  OK:Меню");
  draw4(l0, l1, l2, l3);
}

void uiDrawMenu(bool editing, const char line1[21], const char line2[21], const char line3[21]) {
  char l0[21];
  if (editing) pad20(l0, "МЕНЮ (РЕДАГ.)");
  else         pad20(l0, "МЕНЮ");
  draw4(l0, line1, line2, line3);
}

void uiDrawCalRun(uint16_t totalSec, uint16_t secondsLeft) {
  char l0[21], l1[21], l2[21], l3[21];
  pad20(l0, "КАЛІБРУВАННЯ");
  {
    char b[32];
    snprintf(b, sizeof(b), "Трив: %us", (unsigned)totalSec);
    pad20(l1, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "Зал:  %us", (unsigned)secondsLeft);
    pad20(l2, b);
  }
  pad20(l3, "MENU:Скасувати");
  draw4(l0, l1, l2, l3);
}

void uiDrawCalInputDigits(int32_t ml_x100, uint8_t digitIdx) {
  char l0[21], l1[21], l2[21], l3[21];
  pad20(l0, "КАЛ: ВВЕДИ мл/60с");

  int32_t w = ml_x100 / 100;
  int32_t f = abs(ml_x100) % 100;

  {
    char b[32];
    snprintf(b, sizeof(b), "Знач: %ld.%02ld мл", (long)w, (long)f);
    pad20(l1, b);
  }
  {
    char b[32];
    snprintf(b, sizeof(b), "Розряд: %u", (unsigned)digitIdx);
    pad20(l2, b);
  }
  pad20(l3, "Крути:зм  OK:Далі");
  draw4(l0, l1, l2, l3);
}
