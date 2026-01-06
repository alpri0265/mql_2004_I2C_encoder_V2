#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

static void pad20(char out[21], const char* s) {
  uint8_t i = 0;
  for (; i < 20 && s[i]; i++) out[i] = s[i];
  for (; i < 20; i++) out[i] = ' ';
  out[20] = '\0';
}

static void lcdRow(uint8_t row, const char line[21]) {
  if (!_uiLastValid) _uiSetLastBlank();
  if (memcmp(_uiLast[row], line, 20) == 0) return;

  lcd.setCursor(0, row);
  for (uint8_t i = 0; i < 20; i++) lcd.write(line[i]);
  memcpy(_uiLast[row], line, 21);
}

static const char* matPS(const Settings &S) {
  return (S.material == MAT_STEEL) ? "CTALb" : "ALUMINII";
}
static const char* modePS(const Settings &S) {
  return (S.mode == MODE_CONT) ? "CONT" : "PULSE";
}

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

void uiDrawReady(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20(l0, "GOTOVO");
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "MAT:%s O:%umm", matPS(S), (unsigned)S.cutter_mm);
    pad20(l1, tmp);
  }
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "REZYM:%s", modePS(S));
    pad20(l2, tmp);
  }
  pad20(l3, "HOLD:MENU START:GO");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawWizMaterial(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20(l0, "WIZ: MATERIAL");
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "> %s", matPS(S));
    pad20(l1, tmp);
  }
  pad20(l2, "ROT: select");
  pad20(l3, "OK:Next START:Exit");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawWizDiameter(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20(l0, "WIZ: DIAMETER");
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "> %u mm", (unsigned)S.cutter_mm);
    pad20(l1, tmp);
  }
  pad20(l2, "ROT: select");
  pad20(l3, "OK:Next START:Exit");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawWizRecommend(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100,
                        int32_t potMin_u_x100, int32_t potMax_u_x100) {
  (void)potMin_u_x100; (void)potMax_u_x100;
  char l0[21], l1[21], l2[21], l3[21];

  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "REC:%ld.%02ld", (long)(rec_u_x100/100), (long)(labs(rec_u_x100)%100));
    pad20(l0, tmp);
  }
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "SET:%ld.%02ld", (long)(set_u_x100/100), (long)(labs(set_u_x100)%100));
    pad20(l1, tmp);
  }
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "MODE:%s", modePS(S));
    pad20(l2, tmp);
  }
  pad20(l3, "START:GO");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawRun(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100, bool running) {
  char l0[21], l1[21], l2[21], l3[21];
  (void)S;

  pad20(l0, running ? "RUN" : "STOP");
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "REC:%ld.%02ld", (long)(rec_u_x100/100), (long)(labs(rec_u_x100)%100));
    pad20(l1, tmp);
  }
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "FLOW:%ld.%02ld", (long)(set_u_x100/100), (long)(labs(set_u_x100)%100));
    pad20(l2, tmp);
  }
  pad20(l3, "HOLD:MENU START:STOP");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawMenu(bool editing, const char line1[21], const char line2[21], const char line3[21]) {
  char l0[21];
  pad20(l0, editing ? "MENU (EDIT)" : "MENU");
  lcdRow(0, l0);
  lcdRow(1, line1);
  lcdRow(2, line2);
  lcdRow(3, line3);
}

void uiDrawCalRun(uint16_t totalSec, uint16_t secondsLeft) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20(l0, "CALIBRATION");
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "TOTAL:%us", (unsigned)totalSec);
    pad20(l1, tmp);
  }
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "LEFT:%us", (unsigned)secondsLeft);
    pad20(l2, tmp);
  }
  pad20(l3, "HOLD:BACK");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}

void uiDrawCalInputDigits(int32_t ml_x100, uint8_t digitIdx) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20(l0, "CAL: INPUT ML");
  {
    int32_t w = ml_x100 / 100;
    int32_t f = labs(ml_x100) % 100;
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "VAL:%ld.%02ld", (long)w, (long)f);
    pad20(l1, tmp);
  }
  {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "DIGIT:%u", (unsigned)(digitIdx + 1));
    pad20(l2, tmp);
  }
  pad20(l3, "OK:Next HOLD:Back");

  lcdRow(0, l0); lcdRow(1, l1); lcdRow(2, l2); lcdRow(3, l3);
}
