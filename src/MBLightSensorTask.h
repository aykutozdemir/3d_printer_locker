#pragma once

#include <FsmOS.h>
#include "Constants.h"

class MBLightSensorTask : public Task {
public:
    MBLightSensorTask() {
        set_period(100); // Check every 100ms
    }

protected:
    void on_start() override;
    void step() override;
    void on_msg(const MsgData& msg) override;

private:
    void readMBLightSensor();
    
    bool lastSensorState = false;
};
