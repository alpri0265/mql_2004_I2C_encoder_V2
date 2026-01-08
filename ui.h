#pragma once
#include "types.h"
#include <Arduino.h>

void uiBegin();
void uiClear();

void uiDrawReady(const Settings &S);
void uiDrawWizMaterial(const Settings &S);
void uiDrawWizDiameter(const Settings &S);
void uiDrawWizRecommend(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100, int32_t potMin_u_x100, int32_t potMax_u_x100);
void uiDrawRun(const Settings &S, int32_t rec_u_x100, int32_t set_u_x100, bool running);

void uiDrawMenu(bool editing, const char line1[21], const char line2[21], const char line3[21]);

void uiDrawCalRun(uint16_t totalSec, uint16_t secondsLeft);
void uiDrawCalInputDigits(int32_t ml_x100, uint8_t digitIdx); // digitIdx 0..3

// ✅ НОВЕ: LCD TEST (таблиця символів). base: 0x20,0x30..0xF0
void uiDrawLcdTest(uint8_t base);
void uiDrawLcdTest(uint8_t base);   // base = 0xA0..0xFF

// Test function to display specific Cyrillic letters with their codes
void uiDrawCyrillicTest();