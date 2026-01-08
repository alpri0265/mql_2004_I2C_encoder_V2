#pragma once
#include <avr/pgmspace.h>

// Russian UI strings (stored in PROGMEM to save RAM)
// Using Cyrillic UTF-8 encoding

// === Material names ===
static const char UI_STR_STEEL_UA[] PROGMEM = "Сталь";
static const char UI_STR_ALUMINUM_UA[] PROGMEM = "Алюмин";

// === Mode names ===
static const char UI_STR_CONT_UA[] PROGMEM = "БЕЗПРЕР";
static const char UI_STR_PULSE_UA[] PROGMEM = "ИМПУЛЬС";

// === Screen titles ===
static const char UI_STR_READY_UA[] PROGMEM = "ГОТОВО";
static const char UI_STR_MENU_UA[] PROGMEM = "МЕНЮ";
static const char UI_STR_MENU_EDIT_UA[] PROGMEM = "МЕНЮ (РЕД)";
static const char UI_STR_WIZ_MAT_UA[] PROGMEM = "МАСТЕР: МАТЕРИАЛ";
static const char UI_STR_WIZ_DIA_UA[] PROGMEM = "МАСТЕР: ФРЕЗА O";
static const char UI_STR_WIZ_REC_UA[] PROGMEM = "МАСТЕР: РЕКОМЕНД";
static const char UI_STR_RUN_UA[] PROGMEM = "РАБОТА";
static const char UI_STR_RUN_ON_UA[] PROGMEM = "ВКЛ";
static const char UI_STR_RUN_OFF_UA[] PROGMEM = "ВЫКЛ";
static const char UI_STR_CAL_RUN_UA[] PROGMEM = "КАЛИБРОВКА";
static const char UI_STR_CAL_INPUT_UA[] PROGMEM = "КАЛ: ВВЕДИТЕ мл";

// === Labels ===
static const char UI_STR_MAT_UA[] PROGMEM = "Мат:";
static const char UI_STR_MODE_UA[] PROGMEM = "Реж:";
static const char UI_STR_TURN_CHANGE_UA[] PROGMEM = "Пов: изменить";
static const char UI_STR_OK_NEXT_UA[] PROGMEM = "OK:Далее";
static const char UI_STR_MENU_BACK_UA[] PROGMEM = "МЕНЮ:Назад";
static const char UI_STR_START_TOGGLE_UA[] PROGMEM = "ПУСК:Перекл";
static const char UI_STR_OK_MENU_UA[] PROGMEM = "OK:Меню";
static const char UI_STR_START_RUN_UA[] PROGMEM = "ПУСК:Старт";
static const char UI_STR_OK_MENU_START_UA[] PROGMEM = "OK:Меню  ПУСК:Старт";
static const char UI_STR_MENU_ABORT_UA[] PROGMEM = "МЕНЮ:Отмена";
static const char UI_STR_TURN_CHG_UA[] PROGMEM = "Пов:изм";
static const char UI_STR_OK_NEXT_MENU_UA[] PROGMEM = "OK:Далее МЕНЮ";

// === Calibration ===
static const char UI_STR_TOTAL_UA[] PROGMEM = "Всего:";
static const char UI_STR_LEFT_UA[] PROGMEM = "Осталось:";
static const char UI_STR_VALUE_UA[] PROGMEM = "Значение:";
static const char UI_STR_DIGIT_UA[] PROGMEM = "Разряд:";
static const char UI_STR_ML_UA[] PROGMEM = "мл";

// === Menu items ===
static const char UI_STR_MENU_MATERIAL_UA[] PROGMEM = "Материал:";
static const char UI_STR_MENU_CUTTER_UA[] PROGMEM = "Фреза D:";
static const char UI_STR_MENU_MODE_UA[] PROGMEM = "Режим:";
static const char UI_STR_MENU_PULSE_ON_UA[] PROGMEM = "Имп ВКЛ:";
static const char UI_STR_MENU_PULSE_OFF_UA[] PROGMEM = "Имп ВЫКЛ:";
static const char UI_STR_MENU_KMIN_UA[] PROGMEM = "Kmin:";
static const char UI_STR_MENU_KMAX_UA[] PROGMEM = "Kmax:";
static const char UI_STR_MENU_ALFACTOR_UA[] PROGMEM = "AlКоэф:";
static const char UI_STR_MENU_POT_AVG_UA[] PROGMEM = "ПОТ Среднее:";
static const char UI_STR_MENU_POT_HYST_UA[] PROGMEM = "ПОТ Гист:";
static const char UI_STR_MENU_PUMPGAIN_UA[] PROGMEM = "НасосКоэф:";
static const char UI_STR_MENU_CAL_60_UA[] PROGMEM = "Калибр 60с";
static const char UI_STR_MENU_CAL_120_UA[] PROGMEM = "Калибр 120с";
static const char UI_STR_MENU_CAL_MLU_UA[] PROGMEM = "Кал мл/у:";
static const char UI_STR_MENU_CAL_NONE_UA[] PROGMEM = "(нет)";
static const char UI_STR_MENU_CLEAR_CAL_UA[] PROGMEM = "Очистить калибр";
static const char UI_STR_MENU_SAVE_UA[] PROGMEM = "Сохранить EEPROM";
static const char UI_STR_MENU_DEFAULTS_UA[] PROGMEM = "По умолчанию";
static const char UI_STR_MENU_LANGUAGE_UA[] PROGMEM = "Язык:";
static const char UI_STR_MENU_LANG_EN_UA[] PROGMEM = "АНГ";
static const char UI_STR_MENU_LANG_UA_UA[] PROGMEM = "РУС";
static const char UI_STR_MENU_LCD_TEST_UA[] PROGMEM = "Тест LCD";

// === Units ===
static const char UI_STR_MM_UA[] PROGMEM = "мм";
static const char UI_STR_U_UA[] PROGMEM = "у";
static const char UI_STR_MS_UA[] PROGMEM = "мс";
static const char UI_STR_S_UA[] PROGMEM = "с";
