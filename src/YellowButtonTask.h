#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class YellowButtonTask : public Task
{
public:
    YellowButtonTask();

    void on_start() override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    bool lastButtonState;
    bool buttonPressed;
    Timer8 debounceTimer;     // 50ms debounce - 2 bytes
    Timer16 longPressTimer;   // 1000ms long press - 4 bytes
    bool longPressDetected;
};
