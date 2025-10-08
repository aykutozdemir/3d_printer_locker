#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class BuzzerTask : public Task
{
public:
    BuzzerTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 0; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    enum BuzzerState
    {
        BUZZER_IDLE,
        BUZZER_PLAYING,
        BUZZER_TRIPLE_BEEP,
        BUZZER_ANGRY_CONTINUOUS
    };

    BuzzerState currentState;
    Timer16 soundTimer;  // 1000ms sound duration - 4 bytes
    Timer16 beepTimer;   // Timer for triple-beep pattern - 4 bytes
    unsigned long soundDuration;
    uint16_t currentFrequency;
    uint8_t isPlaying: 1;
    uint8_t beepCount;  // Counter for triple-beep pattern - 1 byte

    // Sound patterns
    void playButtonPress();
    void playPasswordChange();
    void playPasswordAccept();
    void playDoorOpen();
    void playDoorClose();
    void playAngrySound();

    void startSound(uint16_t frequency, unsigned long duration);
    void stopSound();
};
