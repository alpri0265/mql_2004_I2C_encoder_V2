#pragma once
#include <avr/pgmspace.h>

// English UI strings (stored in PROGMEM to save RAM)

// === Material names ===
static const char UI_STR_STEEL_EN[] PROGMEM = "Steel";
static const char UI_STR_ALUMINUM_EN[] PROGMEM = "Alum";

// === Mode names ===
static const char UI_STR_CONT_EN[] PROGMEM = "CONT";
static const char UI_STR_PULSE_EN[] PROGMEM = "PULSE";

// === Screen titles ===
static const char UI_STR_READY_EN[] PROGMEM = "READY";
static const char UI_STR_MENU_EN[] PROGMEM = "MENU";
static const char UI_STR_MENU_EDIT_EN[] PROGMEM = "MENU (EDIT)";
static const char UI_STR_WIZ_MAT_EN[] PROGMEM = "WIZARD: MATERIAL";
static const char UI_STR_WIZ_DIA_EN[] PROGMEM = "WIZARD: CUTTER O";
static const char UI_STR_WIZ_REC_EN[] PROGMEM = "WIZARD: RECOMMEND";
static const char UI_STR_RUN_EN[] PROGMEM = "RUN";
static const char UI_STR_RUN_ON_EN[] PROGMEM = "ON";
static const char UI_STR_RUN_OFF_EN[] PROGMEM = "OFF";
static const char UI_STR_CAL_RUN_EN[] PROGMEM = "CALIBRATION RUN";
static const char UI_STR_CAL_INPUT_EN[] PROGMEM = "CAL: ENTER ml/60s";

// === Labels ===
static const char UI_STR_MAT_EN[] PROGMEM = "Mat:";
static const char UI_STR_MODE_EN[] PROGMEM = "Mode:";
static const char UI_STR_TURN_CHANGE_EN[] PROGMEM = "Turn: change";
static const char UI_STR_OK_NEXT_EN[] PROGMEM = "OK:Next";
static const char UI_STR_MENU_BACK_EN[] PROGMEM = "MENU:Back";
static const char UI_STR_START_TOGGLE_EN[] PROGMEM = "START:Toggle";
static const char UI_STR_OK_MENU_EN[] PROGMEM = "OK:Menu";
static const char UI_STR_START_RUN_EN[] PROGMEM = "START:Run";
static const char UI_STR_OK_MENU_START_EN[] PROGMEM = "OK:Menu  START:Run";
static const char UI_STR_MENU_ABORT_EN[] PROGMEM = "MENU:Abort";
static const char UI_STR_TURN_CHG_EN[] PROGMEM = "Turn:chg";
static const char UI_STR_OK_NEXT_MENU_EN[] PROGMEM = "OK:Next MENU";

// === Calibration ===
static const char UI_STR_TOTAL_EN[] PROGMEM = "Total:";
static const char UI_STR_LEFT_EN[] PROGMEM = "Left :";
static const char UI_STR_VALUE_EN[] PROGMEM = "Value:";
static const char UI_STR_DIGIT_EN[] PROGMEM = "Digit:";
static const char UI_STR_ML_EN[] PROGMEM = "ml";

// === Menu items ===
static const char UI_STR_MENU_MATERIAL_EN[] PROGMEM = "Material:";
static const char UI_STR_MENU_CUTTER_EN[] PROGMEM = "Cutter D:";
static const char UI_STR_MENU_MODE_EN[] PROGMEM = "Mode:";
static const char UI_STR_MENU_PULSE_ON_EN[] PROGMEM = "Pulse ON:";
static const char UI_STR_MENU_PULSE_OFF_EN[] PROGMEM = "Pulse OFF:";
static const char UI_STR_MENU_KMIN_EN[] PROGMEM = "Kmin:";
static const char UI_STR_MENU_KMAX_EN[] PROGMEM = "Kmax:";
static const char UI_STR_MENU_ALFACTOR_EN[] PROGMEM = "AlFactor:";
static const char UI_STR_MENU_POT_AVG_EN[] PROGMEM = "POT Avg N:";
static const char UI_STR_MENU_POT_HYST_EN[] PROGMEM = "POT Hyst:";
static const char UI_STR_MENU_PUMPGAIN_EN[] PROGMEM = "PumpGain:";
static const char UI_STR_MENU_CAL_60_EN[] PROGMEM = "Calibrate 60s";
static const char UI_STR_MENU_CAL_120_EN[] PROGMEM = "Calibrate 120s";
static const char UI_STR_MENU_CAL_MLU_EN[] PROGMEM = "Cal ml/u:";
static const char UI_STR_MENU_CAL_NONE_EN[] PROGMEM = "(none)";
static const char UI_STR_MENU_CLEAR_CAL_EN[] PROGMEM = "Clear calibration";
static const char UI_STR_MENU_SAVE_EN[] PROGMEM = "Save EEPROM";
static const char UI_STR_MENU_DEFAULTS_EN[] PROGMEM = "Load Defaults";
static const char UI_STR_MENU_LANGUAGE_EN[] PROGMEM = "Language:";
static const char UI_STR_MENU_LANG_EN_EN[] PROGMEM = "EN";
static const char UI_STR_MENU_LANG_UA_EN[] PROGMEM = "UA";
static const char UI_STR_MENU_LCD_TEST_EN[] PROGMEM = "LCD Test";

// === Units ===
static const char UI_STR_MM_EN[] PROGMEM = "mm";
static const char UI_STR_U_EN[] PROGMEM = "u";
static const char UI_STR_MS_EN[] PROGMEM = "ms";
static const char UI_STR_S_EN[] PROGMEM = "s";
