#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class ChildLockTask : public Task
{
public:
    ChildLockTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 3; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    uint8_t childLockEngaged: 1; // true = locked (screen/power disabled), false = unlocked
    uint8_t deviceRunning: 1; // true = device is running, false = device is stopped
    Timer16 childLockTimeoutTimer; // 60000ms timeout - 4 bytes

    void releaseChildLock();
    void engageChildLock();
    void updateChildLockState();
};
