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

// Complete mapping table: Unicode -> LCD byte code
// Based on actual LCD test results:
// 0x41 = А
// 0xA0-0xAF = Б, Г, е, Ж, З, И, й, Л, П, У, Ф, Ч, Ш, Ъ, Ы, Э
// 0xB0-0xBF = Ю, Я, б, в, г, е, ж, з, и, й, к, л, м, н, п, т
// 0xC0-0xC7 = ч, ш, ъ, ы, ь, э, ю, я
// User test results: А6-й (0xA6 для заглавной Й), В9-й (0xB9 для строчной й)
static uint8_t unicodeToLcdByte(uint16_t codePoint) {
  // Ё/ё special handling
  if (codePoint == 0x0401) return 0x45; // Ё -> use Е position (0x45 = 'E')
  if (codePoint == 0x0451) return 0xB5; // ё -> е (0xB5)
  // Special check for ь (soft sign) - explicit handling
  if (codePoint == 0x044C) return 0xC4; // ь -> 0xC4
  
  // Complete lookup table based on actual LCD encoding
  switch (codePoint) {
    // Capitals А-Я (U+0410-U+042F)
    case 0x0410: return 0x41; // А -> 0x41 (ASCII 'A')
    case 0x0411: return 0xA0; // Б
    case 0x0412: return 0x42; // В -> use 'B' (0x42)
    case 0x0413: return 0xA1; // Г
    case 0x0414: return 0x44; // Д -> use 'D' (0x44)
    case 0x0415: return 0x45; // Е -> use 'E' (0x45)
    case 0x0416: return 0xA3; // Ж
    case 0x0417: return 0xA4; // З
    case 0x0418: return 0xA5; // И
    case 0x0419: return 0xA6; // Й
    case 0x041A: return 0x4B; // К -> use 'K' (0x4B)
    case 0x041B: return 0xA7; // Л
    case 0x041C: return 0x4D; // М -> use 'M' (0x4D)
    case 0x041D: return 0x48; // Н -> use 'H' (0x48)
    case 0x041E: return 0x4F; // О -> use 'O' (0x4F)
    case 0x041F: return 0xA8; // П
    case 0x0420: return 0x50; // Р -> use 'P' (0x50)
    case 0x0421: return 0x43; // С -> use 'C' (0x43)
    case 0x0422: return 0x54; // Т -> use 'T' (0x54)
    case 0x0423: return 0xA9; // У
    case 0x0424: return 0xAA; // Ф
    case 0x0425: return 0x58; // Х -> use 'X' (0x58)
    case 0x0426: return 0x43; // Ц -> use 'C' (same as С)
    case 0x0427: return 0xAB; // Ч
    case 0x0428: return 0xAC; // Ш
    case 0x0429: return 0xAC; // Щ -> use Ш
    case 0x042A: return 0xAD; // Ъ
    case 0x042B: return 0xAE; // Ы
    case 0x042C: return 0x62; // Ь -> use 'b' (0x62)
    case 0x042D: return 0xAF; // Э
    case 0x042E: return 0xB0; // Ю
    case 0x042F: return 0xB1; // Я
    
    // Lowercase а-я (U+0430-U+044F)
    case 0x0430: return 0x61; // а -> use 'a' (0x61)
    case 0x0431: return 0xB2; // б
    case 0x0432: return 0xB3; // в
    case 0x0433: return 0xB4; // г
    case 0x0434: return 0x64; // д -> use 'd' (0x64)
    case 0x0435: return 0x65; // е -> use 'e' (0x65)
    case 0x0436: return 0xB6; // ж
    case 0x0437: return 0xB7; // з
    case 0x0438: return 0xB8; // и
    case 0x0439: return 0xB9; // й
    case 0x043A: return 0xBA; // к
    case 0x043B: return 0xBB; // л
    case 0x043C: return 0xBC; // м
    case 0x043D: return 0xBD; // н
    case 0x043E: return 0x6F; // о -> use 'o' (0x6F)
    case 0x043F: return 0xBE; // п
    case 0x0440: return 0x70; // р -> use 'p' (0x70)
    case 0x0441: return 0x63; // с -> use 'c' (0x63)
    case 0x0442: return 0xBF; // т
    case 0x0443: return 0x79; // у -> use 'y' (0x79)
    case 0x0444: return 0xAA; // ф -> use Ф code (0xAA) same as capital Ф
    case 0x0445: return 0x78; // х -> use 'x' (0x78)
    case 0x0446: return 0x63; // ц -> use 'c' (same as с)
    case 0x0447: return 0xC0; // ч
    case 0x0448: return 0xC1; // ш
    case 0x0449: return 0xC1; // щ -> use ш
    case 0x044A: return 0xC2; // ъ
    case 0x044B: return 0xC3; // ы
    case 0x044C: return 0xC4; // ь -> 0xC4
    case 0x044D: return 0xC5; // э
    case 0x044E: return 0xC6; // ю
    case 0x044F: return 0xC7; // я
    default: return '?';
  }
}

// Convert UTF-8 Cyrillic to LCD byte encoding
static void utf8ToLcdEncoding(char* dest, const char* src, size_t destSize) {
  size_t i = 0;
  size_t j = 0;
  size_t srcLen = strlen(src);
  
  while (i < srcLen && j < destSize - 1) {
    uint8_t c1 = (uint8_t)src[i];
    
    // ASCII characters
    if (c1 < 128) {
      dest[j++] = c1;
      i += 1;
      continue;
    }
    
    // UTF-8 two-byte sequences
    if (i + 1 >= srcLen) {
      dest[j++] = '?';
      i += 1;
      continue;
    }
    
    uint8_t c2 = (uint8_t)src[i + 1];
    
    // Handle 0xD0 prefix (U+0400-U+043F)
    if (c1 == 0xD0) {
      if (c2 >= 0x80 && c2 <= 0xBF) {
        uint16_t codePoint = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
        dest[j++] = unicodeToLcdByte(codePoint);
        i += 2;
        continue;
      }
    }
    
    // Handle 0xD1 prefix (U+0440-U+04FF) - including "ь" = 0xD1 0x8C
    if (c1 == 0xD1) {
      // Explicit check for "ь" (soft sign) - UTF-8: 0xD1 0x8C = U+044C
      if (c2 == 0x8C) {
        dest[j++] = 0xC4; // ь -> direct mapping to 0xC4
        i += 2;
        continue;
      }
      if (c2 >= 0x80 && c2 <= 0xBF) {
        uint16_t codePoint = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
        dest[j++] = unicodeToLcdByte(codePoint);
        i += 2;
        continue;
      }
    }
    
    // Unknown/unhandled character
    dest[j++] = '?';
    i += 1;
  }
  
  dest[j] = '\0';
}

// Convert UTF-8 string to LCD encoding for display
void uiConvertUtf8ToAscii(char* dest, const char* src, size_t destSize) {
  utf8ToLcdEncoding(dest, src, destSize);
}

void uiPrintAt(uint8_t col, uint8_t row, const char* s) {
  if (!g_lcd) return;
  g_lcd->setCursor(col, row);
  lcdSafePrint(s);
}

// UTF-8 print with transliteration
void uiPrintAtUtf8(uint8_t col, uint8_t row, const char* s) {
  if (!g_lcd) return;
  
  char asciiBuf[64];
  utf8ToLcdEncoding(asciiBuf, s, sizeof(asciiBuf));
  
  g_lcd->setCursor(col, row);
  g_lcd->print(asciiBuf);
}

void uiClearRow(uint8_t row) {
  if (!g_lcd) return;
  g_lcd->setCursor(0, row);
  // 20 spaces for LCD2004
  g_lcd->print(F("                    "));
}
