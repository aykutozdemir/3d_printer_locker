#include "DiagnosticTask.h"

DiagnosticTask::DiagnosticTask() : Task(F("Diagnostic"))
{
    setPeriod(1000); // Check every 1 second (no automatic printing)
}

void DiagnosticTask::on_start()
{
    Serial.println(F("Diag start"));
}

void DiagnosticTask::step()
{
    // Diagnostic task now only runs when requested via serial commands
    // No automatic periodic printing
}

void DiagnosticTask::displayResetInfo()
{
    // Display FsmOS reset information
    ResetInfo resetInfo;
    if (OS.getResetInfo(resetInfo))
    {
        Serial.print(F("RESET(FsmOS): R="));
        Serial.print(resetInfo.resetReason);
        Serial.print(F(", Task="));
        Serial.println(resetInfo.lastTaskId);
    }

    // Display Optiboot reset cause information using integrated FsmOS functionality
    Serial.print(F("RESET(Opti): "));
    uint8_t rawFlags = OS.getResetCauseFlags();
    ResetCause cause = OS.getResetCause();

    Serial.print(cause);
    Serial.print(F(" (0x"));
    Serial.print(rawFlags, HEX);
    Serial.print(F(")"));

    // Show individual flags
    if (rawFlags != 0)
    {
        Serial.print(F(" ["));
        bool first = true;

        if (rawFlags & RESET_CAUSE_POWER_ON)
        {
            if (!first)
            {
                Serial.print(F(", "));
            }
            Serial.print(F("POR"));
            first = false;
        }
        if (rawFlags & RESET_CAUSE_EXTERNAL)
        {
            if (!first)
            {
                Serial.print(F(", "));
            }
            Serial.print(F("EXT"));
            first = false;
        }
        if (rawFlags & RESET_CAUSE_BROWN_OUT)
        {
            if (!first)
            {
                Serial.print(F(", "));
            }
            Serial.print(F("BOR"));
            first = false;
        }
        if (rawFlags & RESET_CAUSE_WATCHDOG)
        {
            if (!first)
            {
                Serial.print(F(", "));
            }
            Serial.print(F("WDT"));
            first = false;
        }

        Serial.print(F("]"));
    }
    Serial.println();
}

void DiagnosticTask::displayTaskStats()
{
    uint8_t taskCount = OS.getTaskCount();
    Serial.print(F("TASKS: "));
    Serial.println(taskCount);

    for (uint8_t i = 0; i < taskCount; i++)
    {
        TaskStats stats;
        if (OS.getTaskStats(i, stats))
        {
            Task *task = OS.getTask(i);
            if (task)
            {
                Serial.print(F("  T"));
                Serial.print(i);
                Serial.print(F(" ("));
                Serial.print(Task::readTaskName(task));
                Serial.print(F("): R="));
                Serial.print(stats.runCount);
                Serial.print(F(", M="));
                Serial.print(stats.maxExecTimeUs);
                Serial.print(F("us, A="));
                if (stats.runCount > 0)
                {
                    Serial.print(stats.totalExecTimeUs / stats.runCount);
                }
                else
                {
                    Serial.print(0);
                }
                Serial.print(F("us, P="));
                Serial.print(task->getPeriod());
                Serial.println(F("ms"));
            }
        }
    }
}

void DiagnosticTask::displayUptime()
{
    uint32_t uptime = OS.now();
    Serial.print(F("UPTIME: "));
    Serial.print(uptime / 1000);
    Serial.print(F("s ("));
    Serial.print(uptime);
    Serial.println(F("ms)"));
}
