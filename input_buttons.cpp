#include "input_buttons.h"
#include "config.h"

// ========== Button implementation ==========

void Button::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, INPUT_PULLUP);
  _stable = digitalRead(_pin);
  _lastRead = _stable;
  _lastChangeMs = millis();
  _pressStartMs = 0;
  _lastRepeatMs = 0;
  _clicked = false;
  _held = false;
  _repeated = false;
  _firstRepeatDone = false;
}

bool Button::poll() {
  _clicked = false;
  _held = false;
  _repeated = false;

  bool r = digitalRead(_pin);  // HIGH = released, LOW = pressed
  uint32_t now = millis();

  // Detect change
  if (r != _lastRead) {
    _lastRead = r;
    _lastChangeMs = now;
  }

  // Debounce
  if ((now - _lastChangeMs) >= BTN_DEBOUNCE_MS) {
    if (_stable != _lastRead) {
      bool prev = _stable;
      _stable = _lastRead;

      if (prev == HIGH && _stable == LOW) {
        // Button just pressed
        _pressStartMs = now;
        _lastRepeatMs = now;
        _firstRepeatDone = false;
      } else if (prev == LOW && _stable == HIGH) {
        // Button released - generate click
        _clicked = true;
      }
    }
  }

  // Auto-repeat while held
  if (_stable == LOW) {
    _held = true;
    
    uint32_t holdTime = now - _pressStartMs;
    
    if (!_firstRepeatDone) {
      // First repeat after delay
      if (holdTime >= BTN_REPEAT_DELAY_MS) {
        _repeated = true;
        _lastRepeatMs = now;
        _firstRepeatDone = true;
      }
    } else {
      // Subsequent repeats at rate
      if ((now - _lastRepeatMs) >= BTN_REPEAT_RATE_MS) {
        _repeated = true;
        _lastRepeatMs = now;
      }
    }
  }

  return _clicked || _repeated;
}

// ========== ButtonsInput implementation ==========

void ButtonsInput::begin(uint8_t pinUp, uint8_t pinDown, uint8_t pinOk, uint8_t pinMenu) {
  _btnUp.begin(pinUp);
  _btnDown.begin(pinDown);
  _btnOk.begin(pinOk);
  _btnMenu.begin(pinMenu);
}

ButtonEvents ButtonsInput::poll() {
  ButtonEvents ev;

  _btnUp.poll();
  _btnDown.poll();
  _btnOk.poll();
  _btnMenu.poll();

  // UP button: step = -1 (move up in menu / decrease value)
  if (_btnUp.clicked() || _btnUp.repeated()) {
    ev.step = -1;
  }

  // DOWN button: step = +1 (move down in menu / increase value)
  if (_btnDown.clicked() || _btnDown.repeated()) {
    ev.step = +1;
  }

  // OK button
  if (_btnOk.clicked()) {
    ev.okClick = true;
  }

  // MENU/BACK button
  if (_btnMenu.clicked()) {
    ev.menuClick = true;
  }

  return ev;
}
