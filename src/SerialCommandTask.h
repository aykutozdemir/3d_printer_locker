#pragma once

#include <Arduino.h>
#include <FsmOS.h>
#include "Constants.h"

class SerialCommandTask : public Task {
public:
    SerialCommandTask();
    
    void on_start() override;
    void step() override;
    
private:
    static const size_t MAX_BUFFER_SIZE = 32;
    char inputBuffer[MAX_BUFFER_SIZE];
    size_t inputLen = 0;
    
    void processCommand(const char* command);
    void printHelp();
    void printTaskStats();
    void printResetInfo();
    void printUptime();
    void printSystemStatus();
    void handleLEDCommand(const char* args);
    void handleKeypadTest();
    void handleBuzzerTest();
    void handleLightCommand(const char* args);
    void handleChildLockCommand(const char* args);
    void handlePasswordCommand(const char* args);
    void handleLEDStateCommand(const char* args);
    void handleFactoryResetCommand();
    void handleSensorStatus();
    void handleMemoryInfo();
    void printUnknownCommand(const char* command);
};
