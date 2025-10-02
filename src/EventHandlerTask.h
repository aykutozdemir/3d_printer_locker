#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class EventHandlerTask : public Task {
public:
    EventHandlerTask();
    
    void on_start() override;
    void on_msg(const MsgData& msg) override;
    void step() override;
    
private:
    bool isDimming; // true when light dimming mode is active
};
