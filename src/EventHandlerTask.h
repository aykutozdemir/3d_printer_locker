#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class EventHandlerTask : public Task
{
public:
    EventHandlerTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    bool isDimming; // true when light dimming mode is active
};
