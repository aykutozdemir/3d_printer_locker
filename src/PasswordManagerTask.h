#pragma once

#include <Arduino.h>
#include <avr/eeprom.h>
#include <FsmOS.h>
#include "Constants.h"

class PasswordManagerTask : public Task
{
public:
    PasswordManagerTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 2; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    enum PasswordState
    {
        PASSWORD_IDLE,
        PASSWORD_ENTERING,
        PASSWORD_CORRECT,
        PASSWORD_WAITING_DOOR_SELECTION,
        PASSWORD_DOOR_SESSION_ACTIVE,  // Session active while doors are open
        PASSWORD_SCREEN_SESSION_ACTIVE, // Session active for screen/power button
        PASSWORD_CHANGE_ENTER,
        PASSWORD_CHANGE_CONFIRM
    };

    PasswordState currentState;
    char enteredPassword[PASSWORD_LENGTH + 1]; // +1 for null terminator
    char correctPassword[PASSWORD_LENGTH + 1];
    char newPasswordBuffer[PASSWORD_LENGTH + 1];
    Timer16 digitTimeoutTimer;  // 3000ms digit timeout - 4 bytes
    Timer16 selectionTimeoutTimer; // 5000ms selection timeout - 4 bytes
    Timer16 screenTimeoutTimer; // 60000ms screen timeout - 4 bytes
    uint8_t digitCount;

    void resetPassword();
    void checkPassword();
    void handleDoorSelection(uint8_t digit);
    void handleYellowShortPress();
    void resetEntryBuffer();
    void loadPasswordFromEEPROM();
    void savePasswordToEEPROM();
};
