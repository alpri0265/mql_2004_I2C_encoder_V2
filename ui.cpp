#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "ui.h"
#include "ui_print.h"
#include "ui_text_en.h"
#include "ui_text_ua.h"
#include "settings.h"
#if defined(__AVR__)
  #include <avr/pgmspace.h>
#endif

// Helper function to get string from PROGMEM based on language
static void uiStrFromProgmem(char* buf, size_t bufSize, const char* enStr, const char* uaStr) {
  const char* selected = (S.uiLang == UILANG_UA) ? uaStr : enStr;
  strncpy_P(buf, selected, bufSize - 1);
  buf[bufSize - 1] = '\0';
}

// Helper macro to select string pointer based on language (for use in functions that handle PROGMEM)
#define UI_STR_PTR(en, ua) ((S.uiLang == UILANG_UA) ? (ua) : (en))

// === LCD instance ===
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 20, 4);

// === cache last drawn lines ===
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

// Pad20 from PROGMEM - convert UTF-8 to single-byte encoding for LCD
static void pad20_P(char out[21], const char* s_P) {
  // Read from PROGMEM into temp buffer
  char temp[64];
  uint8_t i = 0;
  for (; i < sizeof(temp) - 1; i++) {
    char c = pgm_read_byte(s_P + i);
    if (c == '\0') break;
    temp[i] = c;
  }
  temp[i] = '\0';
  
  // Convert UTF-8 to LCD encoding
  char converted[64];
  uiConvertUtf8ToAscii(converted, temp, sizeof(converted));
  
  // Pad to 20 chars
  for (i = 0; i < 20 && converted[i] != '\0'; i++) {
    out[i] = converted[i];
  }
  for (; i < 20; i++) out[i] = ' ';
  out[20] = '\0';
}

static void drawRow(uint8_t row, const char line[21]) {
  if (!lastValid) setLastBlank();
  if (memcmp(last4[row], line, 20) == 0) return;

  lcd.setCursor(0, row);
  // Write bytes directly to LCD (like in LCD test)
  for (uint8_t i = 0; i < 20; i++) {
    lcd.write((uint8_t)line[i]);
  }

  memcpy(last4[row], line, 21);
}

static void draw4(const char l0[21], const char l1[21],
                  const char l2[21], const char l3[21]) {
  drawRow(0, l0);
  drawRow(1, l1);
  drawRow(2, l2);
  drawRow(3, l3);
}

// Create custom Cyrillic characters for LCD (8 custom chars max)
static void createCyrillicChars() {
  // Custom character patterns for Russian Cyrillic
  // We can create up to 8 custom characters
  
  // Ё (U+0401) - E with two dots
  byte yoUpp[8] = { B11111, B10001, B10001, B11111, B10101, B10101, B11111, B00000 };
  // ё (U+0451) - e with two dots  
  byte yoLow[8] = { B00000, B00000, B01110, B10001, B11111, B10000, B01110, B00000 };
  
  // Ж - mapped to custom char 0
  byte zh[8] = { B10101, B10101, B10101, B01110, B10101, B10101, B10101, B00000 };
  // З - mapped to custom char 1
  byte z[8] = { B01110, B10001, B00001, B00110, B00001, B10001, B01110, B00000 };
  // И - mapped to custom char 2
  byte i[8] = { B10001, B10011, B10101, B11001, B10001, B10001, B10001, B00000 };
  // Й - mapped to custom char 3 (И with accent)
  byte iShort[8] = { B00100, B01010, B10011, B10101, B11001, B10001, B10001, B00000 };
  // Х - mapped to custom char 4
  byte h[8] = { B10001, B10001, B01010, B00100, B01010, B10001, B10001, B00000 };
  // Ц - mapped to custom char 5
  byte ts[8] = { B10001, B10001, B10001, B10001, B10001, B11111, B00001, B00000 };
  // Ч - mapped to custom char 6
  byte ch[8] = { B10001, B10001, B10001, B01111, B00001, B00001, B00001, B00000 };
  // Щ - mapped to custom char 7
  byte shch[8] = { B10001, B10001, B10001, B10001, B10001, B11111, B00001, B00001 };
  
  // Note: Standard LCD2004 may not support custom chars properly
  // This is a fallback - actual implementation depends on LCD controller
  // For now, we'll use transliteration which is more reliable
}

// === init / clear ===
void uiBegin() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  uiPrintInit(&lcd);
  // Try to create custom Cyrillic chars (may not work on all displays)
  // createCyrillicChars();
  lcd.clear();
  lastValid = false;
  setLastBlank();
}

void uiClear() {
  lcd.clear();
  lastValid = false;
  setLastBlank();
}

// === helpers ===
static void matStr(char* buf, size_t bufSize, const Settings &S) {
  if (S.material == MAT_STEEL) {
    uiStrFromProgmem(buf, bufSize, UI_STR_STEEL_EN, UI_STR_STEEL_UA);
  } else {
    uiStrFromProgmem(buf, bufSize, UI_STR_ALUMINUM_EN, UI_STR_ALUMINUM_UA);
  }
}

static void modeStr(char* buf, size_t bufSize, const Settings &S) {
  if (S.mode == MODE_CONT) {
    uiStrFromProgmem(buf, bufSize, UI_STR_CONT_EN, UI_STR_CONT_UA);
  } else {
    uiStrFromProgmem(buf, bufSize, UI_STR_PULSE_EN, UI_STR_PULSE_UA);
  }
}

// === READY ===
void uiDrawReady(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20_P(l0, UI_STR_PTR(UI_STR_READY_EN, UI_STR_READY_UA));

  {
    char b[32];
    char matLabelBuf[16], matBuf[16], mmUnitBuf[8];
    uiStrFromProgmem(matLabelBuf, sizeof(matLabelBuf), UI_STR_MAT_EN, UI_STR_MAT_UA);
    matStr(matBuf, sizeof(matBuf), S);
    uiStrFromProgmem(mmUnitBuf, sizeof(mmUnitBuf), UI_STR_MM_EN, UI_STR_MM_UA);
    snprintf(b, sizeof(b), "%s%s  D%u%s", matLabelBuf, matBuf, (unsigned)S.cutter_mm, mmUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  {
    char b[32];
    char modeLabelBuf[16], modeBuf[16];
    uiStrFromProgmem(modeLabelBuf, sizeof(modeLabelBuf), UI_STR_MODE_EN, UI_STR_MODE_UA);
    modeStr(modeBuf, sizeof(modeBuf), S);
    snprintf(b, sizeof(b), "%s%s", modeLabelBuf, modeBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l2, converted);
  }

  pad20_P(l3, UI_STR_PTR(UI_STR_OK_MENU_START_EN, UI_STR_OK_MENU_START_UA));
  draw4(l0, l1, l2, l3);
}

// === WIZARD ===
void uiDrawWizMaterial(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20_P(l0, UI_STR_PTR(UI_STR_WIZ_MAT_EN, UI_STR_WIZ_MAT_UA));

  {
    char b[32];
    char matBuf[16];
    matStr(matBuf, sizeof(matBuf), S);
    snprintf(b, sizeof(b), "> %s", matBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  pad20_P(l2, UI_STR_PTR(UI_STR_TURN_CHANGE_EN, UI_STR_TURN_CHANGE_UA));
  {
    char b[32];
    char okNextBuf[16], menuBackBuf[16];
    uiStrFromProgmem(okNextBuf, sizeof(okNextBuf), UI_STR_OK_NEXT_EN, UI_STR_OK_NEXT_UA);
    uiStrFromProgmem(menuBackBuf, sizeof(menuBackBuf), UI_STR_MENU_BACK_EN, UI_STR_MENU_BACK_UA);
    snprintf(b, sizeof(b), "%s  %s", okNextBuf, menuBackBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l3, converted);
  }
  draw4(l0, l1, l2, l3);
}

void uiDrawWizDiameter(const Settings &S) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20_P(l0, UI_STR_PTR(UI_STR_WIZ_DIA_EN, UI_STR_WIZ_DIA_UA));

  {
    char b[32];
    char mmUnitBuf[8];
    uiStrFromProgmem(mmUnitBuf, sizeof(mmUnitBuf), UI_STR_MM_EN, UI_STR_MM_UA);
    snprintf(b, sizeof(b), "> %u%s", (unsigned)S.cutter_mm, mmUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  pad20_P(l2, UI_STR_PTR(UI_STR_TURN_CHANGE_EN, UI_STR_TURN_CHANGE_UA));
  {
    char b[32];
    char okNextBuf[16], menuBackBuf[16];
    uiStrFromProgmem(okNextBuf, sizeof(okNextBuf), UI_STR_OK_NEXT_EN, UI_STR_OK_NEXT_UA);
    uiStrFromProgmem(menuBackBuf, sizeof(menuBackBuf), UI_STR_MENU_BACK_EN, UI_STR_MENU_BACK_UA);
    snprintf(b, sizeof(b), "%s  %s", okNextBuf, menuBackBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l3, converted);
  }
  draw4(l0, l1, l2, l3);
}

void uiDrawWizRecommend(const Settings &S,
                        int32_t rec_u_x100,
                        int32_t set_u_x100,
                        int32_t potMin_u_x100,
                        int32_t potMax_u_x100) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20_P(l0, UI_STR_PTR(UI_STR_WIZ_REC_EN, UI_STR_WIZ_REC_UA));

  {
    char b[32];
    char uUnitBuf[8];
    uiStrFromProgmem(uUnitBuf, sizeof(uUnitBuf), UI_STR_U_EN, UI_STR_U_UA);
    snprintf(b, sizeof(b), "Rec: %ld.%02ld %s",
             (long)(rec_u_x100 / 100),
             (long)(abs(rec_u_x100) % 100),
             uUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  {
    char b[32];
    char uUnitBuf[8];
    uiStrFromProgmem(uUnitBuf, sizeof(uUnitBuf), UI_STR_U_EN, UI_STR_U_UA);
    snprintf(b, sizeof(b), "Set: %ld.%02ld %s",
             (long)(set_u_x100 / 100),
             (long)(abs(set_u_x100) % 100),
             uUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l2, converted);
  }

  {
    char b[32];
    snprintf(b, sizeof(b), "POT:%ld..%ld",
             (long)(potMin_u_x100 / 100),
             (long)(potMax_u_x100 / 100));
    pad20(l3, b);
  }

  draw4(l0, l1, l2, l3);
}

// === RUN ===
void uiDrawRun(const Settings &S,
               int32_t rec_u_x100,
               int32_t set_u_x100,
               bool running) {
  char l0[21], l1[21], l2[21], l3[21];

  {
    char b[32];
    char runLabelBuf[16], onBuf[16], offBuf[16], modeBuf[16];
    uiStrFromProgmem(runLabelBuf, sizeof(runLabelBuf), UI_STR_RUN_EN, UI_STR_RUN_UA);
    uiStrFromProgmem(onBuf, sizeof(onBuf), UI_STR_RUN_ON_EN, UI_STR_RUN_ON_UA);
    uiStrFromProgmem(offBuf, sizeof(offBuf), UI_STR_RUN_OFF_EN, UI_STR_RUN_OFF_UA);
    snprintf(b, sizeof(b), "%s: %s", runLabelBuf, running ? onBuf : offBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l0, converted);
  }

  {
    char b[32];
    char modeBuf[16];
    modeStr(modeBuf, sizeof(modeBuf), S);
    snprintf(b, sizeof(b), "Rec:%ld.%02ld  %s",
             (long)(rec_u_x100 / 100),
             (long)(abs(rec_u_x100) % 100),
             modeBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  {
    char b[32];
    snprintf(b, sizeof(b), "Set:%ld.%02ld  D:%u",
             (long)(set_u_x100 / 100),
             (long)(abs(set_u_x100) % 100),
             (unsigned)S.cutter_mm);
    pad20(l2, b);
  }

  {
    char b[32];
    char startToggleBuf[16], okMenuBuf[16];
    uiStrFromProgmem(startToggleBuf, sizeof(startToggleBuf), UI_STR_START_TOGGLE_EN, UI_STR_START_TOGGLE_UA);
    uiStrFromProgmem(okMenuBuf, sizeof(okMenuBuf), UI_STR_OK_MENU_EN, UI_STR_OK_MENU_UA);
    snprintf(b, sizeof(b), "%s  %s", startToggleBuf, okMenuBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l3, converted);
  }
  draw4(l0, l1, l2, l3);
}

// === MENU ===
void uiDrawMenu(bool editing,
                const char line1[21],
                const char line2[21],
                const char line3[21]) {
  char l0[21];
  const char* menuTitle_P = editing 
    ? UI_STR_PTR(UI_STR_MENU_EDIT_EN, UI_STR_MENU_EDIT_UA)
    : UI_STR_PTR(UI_STR_MENU_EN, UI_STR_MENU_UA);
  pad20_P(l0, menuTitle_P);
  draw4(l0, line1, line2, line3);
}

// === CALIBRATION ===
void uiDrawCalRun(uint16_t totalSec, uint16_t secondsLeft) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20_P(l0, UI_STR_PTR(UI_STR_CAL_RUN_EN, UI_STR_CAL_RUN_UA));

  {
    char b[32];
    char totalLabelBuf[16], sUnitBuf[8];
    uiStrFromProgmem(totalLabelBuf, sizeof(totalLabelBuf), UI_STR_TOTAL_EN, UI_STR_TOTAL_UA);
    uiStrFromProgmem(sUnitBuf, sizeof(sUnitBuf), UI_STR_S_EN, UI_STR_S_UA);
    snprintf(b, sizeof(b), "%s %u%s", totalLabelBuf, (unsigned)totalSec, sUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  {
    char b[64];  // Increased from 32 to 64 to ensure full string fits
    char leftLabelBuf[32], sUnitBuf[8];  // Increased leftLabelBuf from 16 to 32 bytes
    uiStrFromProgmem(leftLabelBuf, sizeof(leftLabelBuf), UI_STR_LEFT_EN, UI_STR_LEFT_UA);
    uiStrFromProgmem(sUnitBuf, sizeof(sUnitBuf), UI_STR_S_EN, UI_STR_S_UA);
    snprintf(b, sizeof(b), "%s %u%s", leftLabelBuf, (unsigned)secondsLeft, sUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l2, converted);
  }

  pad20_P(l3, UI_STR_PTR(UI_STR_MENU_ABORT_EN, UI_STR_MENU_ABORT_UA));
  draw4(l0, l1, l2, l3);
}

void uiDrawCalInputDigits(int32_t ml_x100, uint8_t digitIdx) {
  char l0[21], l1[21], l2[21], l3[21];

  pad20_P(l0, UI_STR_PTR(UI_STR_CAL_INPUT_EN, UI_STR_CAL_INPUT_UA));

  int32_t w = ml_x100 / 100;
  int32_t f = abs(ml_x100) % 100;

  {
    char b[64];  // Increased from 32 to 64 to ensure full string fits
    char valueLabelBuf[32], mlUnitBuf[8];  // Increased valueLabelBuf from 16 to 32 bytes
    uiStrFromProgmem(valueLabelBuf, sizeof(valueLabelBuf), UI_STR_VALUE_EN, UI_STR_VALUE_UA);
    uiStrFromProgmem(mlUnitBuf, sizeof(mlUnitBuf), UI_STR_ML_EN, UI_STR_ML_UA);
    snprintf(b, sizeof(b), "%s %ld.%02ld %s", valueLabelBuf, (long)w, (long)f, mlUnitBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l1, converted);
  }

  {
    char b[64];  // Increased from 32 to 64
    char digitLabelBuf[32];  // Increased from 16 to 32 bytes
    uiStrFromProgmem(digitLabelBuf, sizeof(digitLabelBuf), UI_STR_DIGIT_EN, UI_STR_DIGIT_UA);
    snprintf(b, sizeof(b), "%s %u", digitLabelBuf, (unsigned)digitIdx);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l2, converted);
  }

  {
    char b[64];  // Increased from 32 to 64
    char turnChgBuf[32], okNextMenuBuf[32];  // Increased from 16 to 32 bytes
    uiStrFromProgmem(turnChgBuf, sizeof(turnChgBuf), UI_STR_TURN_CHG_EN, UI_STR_TURN_CHG_UA);
    uiStrFromProgmem(okNextMenuBuf, sizeof(okNextMenuBuf), UI_STR_OK_NEXT_MENU_EN, UI_STR_OK_NEXT_MENU_UA);
    snprintf(b, sizeof(b), "%s %s", turnChgBuf, okNextMenuBuf);
    
    // Convert UTF-8 to LCD encoding before padding
    char converted[64];
    uiConvertUtf8ToAscii(converted, b, sizeof(converted));
    pad20(l3, converted);
  }
  draw4(l0, l1, l2, l3);
}

static void uiPrintHex2(uint8_t v) {
  static const char hexdig[] = "0123456789ABCDEF";
  char a = hexdig[(v >> 4) & 0x0F];
  char b = hexdig[v & 0x0F];
  lcd.write(a);
  lcd.write(b);
}

static void uiDrawLcdTestRow(uint8_t row, uint8_t base) {
  lcd.setCursor(0, row);

  // Show hex code like "80:"
  uiPrintHex2(base);
  lcd.write(':');
  lcd.write(' ');

  // Display 16 characters (base..base+15)
  for (uint8_t i = 0; i < 16; i++) {
    lcd.write((uint8_t)(base + i));
  }
}

void uiDrawLcdTest(uint8_t base) {
  lcd.clear();
  uiDrawLcdTestRow(0, base);
  uiDrawLcdTestRow(1, (uint8_t)(base + 0x10));
  uiDrawLcdTestRow(2, (uint8_t)(base + 0x20));
  uiDrawLcdTestRow(3, (uint8_t)(base + 0x30));
}

// Test function to display specific Cyrillic letters with their byte codes
// This helps determine exact encoding for each letter
void uiDrawCyrillicTest() {
  lcd.clear();
  
  // Line 0: Test word "ГОТОВО" with different byte codes
  lcd.setCursor(0, 0);
  lcd.write((uint8_t)0x80); // Try А
  lcd.write((uint8_t)0x81); // Try Б
  lcd.write((uint8_t)0x82); // Try В
  lcd.write((uint8_t)0x83); // Try Г
  lcd.write((uint8_t)0x84); // Try Д
  lcd.write((uint8_t)0x85); // Try Е
  lcd.write(' ');
  lcd.write((uint8_t)0x83); // Г
  lcd.write((uint8_t)0x8E); // О
  lcd.write((uint8_t)0x92); // Т
  lcd.write((uint8_t)0x8E); // О
  lcd.write((uint8_t)0x82); // В
  lcd.write((uint8_t)0x8E); // О
  
  // Line 1: Show codes for Г-О-Т-О-В-О
  lcd.setCursor(0, 1);
  lcd.print("G 0x83 O 0x8E T 0x92");
  
  // Line 2: Test lowercase
  lcd.setCursor(0, 2);
  lcd.write((uint8_t)0xA1); // Try а
  lcd.write((uint8_t)0xA2); // Try б
  lcd.write((uint8_t)0xA3); // Try в
  lcd.write((uint8_t)0xA4); // Try г
  lcd.write((uint8_t)0xA5); // Try д
  lcd.write((uint8_t)0xA6); // Try е
  lcd.write(' ');
  lcd.write((uint8_t)0xB0); // Try п
  lcd.write((uint8_t)0xB1); // Try р
  lcd.write((uint8_t)0xB8); // Try ч (if from test)
  
  // Line 3: Instructions
  lcd.setCursor(0, 3);
  lcd.print("Check codes above");
}
