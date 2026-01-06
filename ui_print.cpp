#include "ui_print.h"
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C* g_lcd = nullptr;

void uiPrintInit(LiquidCrystal_I2C* lcd) {
  g_lcd = lcd;
}

static void lcdSafePrint(const char* s) {
  if (!g_lcd || !s) return;
  g_lcd->print(s);
}

void uiPrintAt(uint8_t col, uint8_t row, const char* s) {
  if (!g_lcd) return;
  g_lcd->setCursor(col, row);
  lcdSafePrint(s);
}

// TEMP: UTF-8 path falls back to ASCII for now.
// Next step we'll implement UTF-8 decode + Cyrillic mapping table.
void uiPrintAtUtf8(uint8_t col, uint8_t row, const char* s) {
  uiPrintAt(col, row, s);
}

void uiClearRow(uint8_t row) {
  if (!g_lcd) return;
  g_lcd->setCursor(0, row);
  // 20 spaces for LCD2004
  g_lcd->print(F("                    "));
}
