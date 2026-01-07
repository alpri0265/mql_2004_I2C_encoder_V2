#pragma once
#include <Arduino.h>

void lcdTestEnter(uint8_t base = 0x20);
void lcdTestExit();
bool lcdTestIsActive();

void lcdTestOnOk();
void lcdTestOnMenu();
void lcdTestOnEnc(int8_t dir);

void lcdTestDraw();
