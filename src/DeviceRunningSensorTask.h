#pragma once

#include <FsmOS.h>
#include "Constants.h"

class DeviceRunningSensorTask : public Task {
public:
    DeviceRunningSensorTask() {
        set_period(200); // Check every 200ms
    }

protected:
    void on_start() override;
    void step() override;
    void on_msg(const MsgData& msg) override;

private:
    void readDeviceRunningSensor();
    
    bool lastDeviceRunningState = false;
};
