#pragma once
#include <Arduino.h>

// Button with debounce and auto-repeat support
class Button {
public:
  void begin(uint8_t pin);
  
  // Call in loop, returns true if button action detected
  bool poll();
  
  // Check button state after poll()
  bool clicked() const { return _clicked; }
  bool held() const { return _held; }
  bool repeated() const { return _repeated; }
  bool isPressed() const { return _stable == LOW; }

private:
  uint8_t _pin = 0;
  bool _lastRead = HIGH;
  bool _stable = HIGH;
  uint32_t _lastChangeMs = 0;
  uint32_t _pressStartMs = 0;
  uint32_t _lastRepeatMs = 0;
  bool _clicked = false;
  bool _held = false;
  bool _repeated = false;
  bool _firstRepeatDone = false;
};

// Buttons controller for 4-button navigation
struct ButtonEvents {
  int8_t step = 0;      // +1 (DOWN pressed) / -1 (UP pressed)
  bool okClick = false;
  bool menuClick = false;
};

class ButtonsInput {
public:
  void begin(uint8_t pinUp, uint8_t pinDown, uint8_t pinOk, uint8_t pinMenu);
  ButtonEvents poll();

private:
  Button _btnUp;
  Button _btnDown;
  Button _btnOk;
  Button _btnMenu;
};
