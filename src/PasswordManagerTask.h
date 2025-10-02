#pragma once

#include <Arduino.h>
#include <avr/eeprom.h>
#include <FsmOS.h>
#include "Constants.h"

class PasswordManagerTask : public Task {
public:
    PasswordManagerTask();
    
    void on_start() override;
    void on_msg(const MsgData& msg) override;
    void step() override;
    
private:
    enum PasswordState {
        PASSWORD_IDLE,
        PASSWORD_ENTERING,
        PASSWORD_CORRECT,
        PASSWORD_WAITING_DOOR_SELECTION,
        PASSWORD_CHANGE_ENTER,
        PASSWORD_CHANGE_CONFIRM
    };
    
    PasswordState currentState;
    char enteredPassword[PASSWORD_LENGTH + 1]; // +1 for null terminator
    char correctPassword[PASSWORD_LENGTH + 1];
    char newPasswordBuffer[PASSWORD_LENGTH + 1];
    unsigned long lastDigitTime;
    uint8_t digitCount;
    
    void resetPassword();
    void checkPassword();
    void handleDoorSelection(uint8_t digit);
    void handleYellowShortPress();
    void resetEntryBuffer();
    void loadPasswordFromEEPROM();
    void savePasswordToEEPROM();
};
