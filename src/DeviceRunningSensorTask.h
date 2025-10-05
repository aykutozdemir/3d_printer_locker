#pragma once

#include <FsmOS.h>
#include "Constants.h"

class DeviceRunningSensorTask : public Task
{
public:
    DeviceRunningSensorTask() : Task(F("DeviceRunning"))
    {
        setPeriod(200); // Check every 200ms
    }
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

protected:
    void on_start() override;
    void step() override;
    void on_msg(const MsgData &msg) override;

private:
    void readDeviceRunningSensor();

    bool lastDeviceRunningState = false;
};
