#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class KeypadTask : public Task
{
public:
    KeypadTask();

    void on_start() override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    struct KeyState
    {
        uint8_t lastState: 1;
        uint8_t currentState: 1;
        Timer8 debounceTimer;     // 50ms debounce - 2 bytes
        Timer16 longPressTimer;   // 1000ms long press - 4 bytes
        uint8_t debounced: 1;
        uint8_t longReported: 1;
    };

    KeyState keys[4]; // For keys 1, 2, 3, 4

    void checkKey(uint8_t keyIndex, uint8_t pin, uint8_t eventType);
    void handleKeyPress(uint8_t keyIndex, uint8_t eventType);
    void handleKeyLongPress(uint8_t keyIndex, uint8_t eventType);
};
