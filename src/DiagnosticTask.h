#pragma once

#include <Arduino.h>
#include <FsmOS.h>

class DiagnosticTask : public Task
{
public:
    DiagnosticTask();

    void on_start() override;
    void step() override;
    uint8_t getMaxMessageBudget() const override { return 1; }
    uint16_t getTaskStructSize() const override { return sizeof(*this); }

private:
    void displayResetInfo();
    void displayTaskStats();
    void displayUptime();
};
