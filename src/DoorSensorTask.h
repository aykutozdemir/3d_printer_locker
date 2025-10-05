#pragma once

#include <FsmOS.h>
#include "Constants.h"

class DoorSensorTask : public Task
{
public:
    DoorSensorTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 2; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    uint8_t frontDoorState: 1;
    uint8_t topDoorState: 1;
    uint8_t lastFrontDoorState: 1;
    uint8_t lastTopDoorState: 1;
    unsigned long lastDebounceTime;
    static const unsigned long SENSOR_DEBOUNCE_TIME_MS = 50;

    void readDoorSensors();
    void publishDoorEvents();
};
