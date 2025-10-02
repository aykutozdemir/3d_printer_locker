#pragma once

#include <Arduino.h>
#include <FsmOS.h>

class DiagnosticTask : public Task {
public:
    DiagnosticTask();
    
    void on_start() override;
    void step() override;
    
private:
    void displayResetInfo();
    void displayTaskStats();
    void displayUptime();
};
