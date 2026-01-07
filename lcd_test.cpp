#include "lcd_test.h"
#include "ui.h"

static bool    gActive = false;
static uint8_t gBase   = 0x20;

void lcdTestEnter(uint8_t base) {
  gActive = true;
  gBase = base;
  lcdTestDraw();
}

void lcdTestExit() {
  gActive = false;
}

bool lcdTestIsActive() {
  return gActive;
}

void lcdTestDraw() {
  if (!gActive) return;
  uiDrawLcdTest(gBase);
}

void lcdTestOnOk()   { if (gActive) lcdTestExit(); }
void lcdTestOnMenu() { if (gActive) lcdTestExit(); }

void lcdTestOnEnc(int8_t dir) {
  if (!gActive) return;
  if (dir > 0) gBase = (uint8_t)(gBase + 0x10);
  else if (dir < 0) gBase = (uint8_t)(gBase - 0x10);
  lcdTestDraw();
}
