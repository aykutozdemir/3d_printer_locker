#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class StatusLEDTask : public Task {
public:
    StatusLEDTask();
    
    void on_start() override;
    void on_msg(const MsgData& msg) override;
    void step() override;
    
private:
    enum LEDState {
        LED_LOCKED,        // Red solid
        LED_UNLOCKED,      // Green solid
        LED_TO_BE_LOCKED,  // Red blinking
        LED_TO_BE_OPENED,  // Green blinking
        LED_CHILD_UNLOCKED // Alternate pattern when child lock disabled
    };
    
    LEDState currentState;
    bool redLedState;
    bool greenLedState;
    unsigned long lastBlinkTime;
    static const unsigned long BLINK_INTERVAL_MS = 500; // 500ms blink interval
    bool altToggle; // used for alternating pattern in CHILD_UNLOCKED state
    
    void setLEDs(bool red, bool green);
    void updateLEDs();
};
