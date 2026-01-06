#pragma once
#include <Arduino.h>

// Forward-declare LiquidCrystal_I2C to avoid including heavy headers here
class LiquidCrystal_I2C;

// Call once in setup after lcd.begin()
void uiPrintInit(LiquidCrystal_I2C* lcd);

// ASCII print (current behavior)
void uiPrintAt(uint8_t col, uint8_t row, const char* s);

// UTF-8 print (for Ukrainian). For now it will fall back to ASCII until mapping is added.
void uiPrintAtUtf8(uint8_t col, uint8_t row, const char* s);

// Helpers
void uiClearRow(uint8_t row);
