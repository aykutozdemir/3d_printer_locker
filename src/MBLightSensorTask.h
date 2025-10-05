#pragma once

#include <FsmOS.h>
#include "Constants.h"

class MBLightSensorTask : public Task
{
public:
    MBLightSensorTask() : Task(F("MBLightSensor"))
    {
        setPeriod(100); // Check every 100ms
    }
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

protected:
    void on_start() override;
    void step() override;
    void on_msg(const MsgData &msg) override;

private:
    void readMBLightSensor();

    bool lastSensorState = false;
};
