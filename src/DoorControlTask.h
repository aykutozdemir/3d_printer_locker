#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class DoorControlTask : public Task {
public:
    DoorControlTask();
    
    void on_start() override;
    void on_msg(const MsgData& msg) override;
    void step() override;
    
private:
    uint8_t frontDoorReleased:1;
    uint8_t topDoorReleased:1;
    uint8_t frontDoorOpened:1; // Track if front door is physically opened
    uint8_t topDoorOpened:1;   // Track if top door is physically opened
    uint8_t waitingForDoorOpen:1; // Track if we're waiting for door to be opened
    uint8_t lastLEDState:1; // Track last LED state to avoid duplicate messages
    unsigned long frontDoorOpenTime; // Time when front door was opened
    unsigned long topDoorOpenTime;   // Time when top door was opened
    unsigned long frontDoorCloseTime; // Time when front door was closed
    unsigned long topDoorCloseTime;   // Time when top door was closed
    uint8_t frontDoorNeedsReengage:1; // Flag to re-engage front door magnet
    uint8_t topDoorNeedsReengage:1;   // Flag to re-engage top door magnet
    uint8_t unauthorizedAccessActive:1; // Flag to track if angry sound is playing
    static const unsigned long MAGNET_DELAY_MS = 1500; // 1.5 second delay
    static const unsigned long REENGAGE_DELAY_MS = 100; // 100ms delay for re-engagement
    
    void releaseFrontDoor();
    void releaseTopDoor();
    void releaseBothDoors();
    void lockAllDoors();
    void updateStatusLEDs();
    void handleDoorSensorEvent(uint8_t eventType);
};
