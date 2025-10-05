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
        BUZZER_ANGRY_CONTINUOUS
    };

    BuzzerState currentState;
    Timer16 soundTimer;  // 1000ms sound duration - 4 bytes
    unsigned long soundDuration;
    uint16_t currentFrequency;
    uint8_t isPlaying: 1;

    // Sound patterns
    void playButtonPress();
    void playKeypadPress();
    void playWrongPassword();
    void playCorrectPassword();
    void playDoorReleased();
    void playDoorClosed();
    void playAngrySound();
    void startAngrySound();
    void stopAngrySound();
    void playTopDoorSelected();
    void playFrontDoorSelected();
    void playBothDoorsSelected();
    void playChildLockSelected();

    void startSound(uint16_t frequency, unsigned long duration);
    void stopSound();
};
