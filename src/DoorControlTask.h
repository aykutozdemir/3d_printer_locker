#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class DoorControlTask : public Task
{
public:
    DoorControlTask();

    void on_start() override;
    void on_msg(const MsgData &msg) override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 3; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    uint8_t frontDoorReleased: 1;
    uint8_t topDoorReleased: 1;
    uint8_t frontDoorOpened: 1; // Track if front door is physically opened
    uint8_t topDoorOpened: 1;  // Track if top door is physically opened
    uint8_t waitingForDoorOpen: 1; // Track if we're waiting for door to be opened
    uint8_t lastLEDState: 1; // Track last LED state to avoid duplicate messages
    Timer16 frontDoorOpeningTimer; // 5000ms opening delay - 4 bytes
    Timer16 topDoorOpeningTimer;   // 5000ms opening delay - 4 bytes
    Timer16 frontDoorMagnetTimer; // 3000ms magnet hold - 4 bytes
    Timer16 topDoorMagnetTimer;   // 3000ms magnet hold - 4 bytes
    Timer8 frontDoorCloseTimer; // 100ms reengage delay - 2 bytes
    Timer8 topDoorCloseTimer;   // 100ms reengage delay - 2 bytes
    uint8_t frontDoorNeedsReengage: 1; // Flag to re-engage front door magnet
    uint8_t topDoorNeedsReengage: 1;  // Flag to re-engage top door magnet
    uint8_t unauthorizedAccessActive: 1; // Flag to track if angry sound is playing
    uint8_t frontDoorReengageLogged: 1; // Flag to prevent duplicate re-engage logging
    uint8_t topDoorReengageLogged: 1;  // Flag to prevent duplicate re-engage logging
    uint8_t startupDetectionComplete: 1; // Flag to track if startup state detection is complete
    static const unsigned long MAGNET_DELAY_MS = 1500; // 1.5 second delay
    static const unsigned long REENGAGE_DELAY_MS = 100; // 100ms delay for re-engagement

    void releaseFrontDoor();
    void releaseTopDoor();
    void releaseBothDoors();
    void lockAllDoors();
    void handleStartupDoorDetection(uint8_t eventType);
    void updateStatusLEDs();
    void handleDoorSensorEvent(uint8_t eventType);
};
