#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class ChildLockTask : public Task {
public:
    ChildLockTask();
    
    void on_start() override;
    void on_msg(const MsgData& msg) override;
    void step() override;
    
private:
    uint8_t childLockEngaged:1; // true = locked (screen/power disabled), false = unlocked
    uint8_t deviceRunning:1; // true = device is running, false = device is stopped
    unsigned long childLockReleaseTime; // timestamp when released for timeout tracking
    
    void releaseChildLock();
    void engageChildLock();
    void updateChildLockState();
};
