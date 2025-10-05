#pragma once

#include <Arduino.h>
#include <avr/eeprom.h>
#include <FsmOS.h>
#include "Constants.h"

class LightTask : public Task
{
public:
    LightTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    enum LightState
    {
        LIGHT_OFF,
        LIGHT_ON,
        LIGHT_DIMMING
    };

    LightState currentState;
    bool lightOn;
    uint8_t currentDimLevel;      // 0-100%
    uint8_t savedDimLevel;        // Saved dim level for permanent storage
    Timer16 dimStepTimer;  // 1000ms dim step interval - 4 bytes
    bool dimIncreasing;
    bool manualOverride;          // True when user manually controls light (prevents MB sensor override)

    static const unsigned long DIM_STEP_INTERVAL_MS = 1000; // 1 second per step
    static const uint8_t DIM_STEP_PERCENT = 10;             // 10% per step
    static const uint8_t MAX_DIM_LEVEL = 100;               // 100% maximum

    void setLightState(bool on);
    void setDimLevel(uint8_t level);
    void saveDimLevel();
    void loadDimLevel();
    uint16_t dimLevelToPWM(uint8_t level);
};
