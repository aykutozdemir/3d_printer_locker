#include "DiagnosticTask.h"

DiagnosticTask::DiagnosticTask() {
    set_period(1000); // Check every 1 second (no automatic printing)
}

void DiagnosticTask::on_start() {
    Serial.println(F("DIAGNOSTIC: Task started"));
}

void DiagnosticTask::step() {
    // Diagnostic task now only runs when requested via serial commands
    // No automatic periodic printing
}

void DiagnosticTask::displayResetInfo() {
    ResetInfo resetInfo;
    if (OS.get_reset_info(resetInfo)) {
        Serial.print(F("RESET_INFO: Reset Reason="));
        Serial.print(resetInfo.reset_reason);
        Serial.print(F(", Last Task="));
        Serial.println(resetInfo.last_task_id);
    }
}

void DiagnosticTask::displayTaskStats() {
    uint8_t taskCount = OS.get_task_count();
    Serial.print(F("TASK_STATS: Total tasks="));
    Serial.println(taskCount);
    
    for (uint8_t i = 0; i < taskCount; i++) {
        TaskStats stats;
        if (OS.get_task_stats(i, stats)) {
            Task* task = OS.get_task(i);
            if (task) {
                Serial.print(F("  Task "));
                Serial.print(i);
                Serial.print(F(" ("));
                Serial.print(task->get_name());
                Serial.print(F("): Runs="));
                Serial.print(stats.run_count);
                Serial.print(F(", MaxTime="));
                Serial.print(stats.max_exec_time_us);
                Serial.print(F("us, AvgTime="));
                if (stats.run_count > 0) {
                    Serial.print(stats.total_exec_time_us / stats.run_count);
                } else {
                    Serial.print(0);
                }
                Serial.print(F("us, Period="));
                Serial.print(task->get_period());
                Serial.println(F("ms"));
            }
        }
    }
}

void DiagnosticTask::displayUptime() {
    uint32_t uptime = OS.now();
    Serial.print(F("UPTIME: "));
    Serial.print(uptime / 1000);
    Serial.print(F("s ("));
    Serial.print(uptime);
    Serial.println(F("ms)"));
}
