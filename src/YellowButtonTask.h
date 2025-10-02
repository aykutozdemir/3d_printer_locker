#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class YellowButtonTask : public Task {
public:
    YellowButtonTask();
    
    void on_start() override;
    void step() override;
    
private:
    bool lastButtonState;
    bool buttonPressed;
    unsigned long pressStartTime;
    bool longPressDetected;
};
