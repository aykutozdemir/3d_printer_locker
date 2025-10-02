#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class KeypadTask : public Task {
public:
    KeypadTask();
    
    void on_start() override;
    void step() override;
    
private:
    struct KeyState {
        uint8_t lastState:1;
        uint8_t currentState:1;
        unsigned long lastPressTime;
        uint8_t debounced:1;
        uint8_t longReported:1;
    };
    
    KeyState keys[4]; // For keys 1, 2, 3, 4
    
    void checkKey(uint8_t keyIndex, uint8_t pin, uint8_t eventType);
    void handleKeyPress(uint8_t keyIndex, uint8_t eventType);
    void handleKeyLongPress(uint8_t keyIndex, uint8_t eventType);
};
