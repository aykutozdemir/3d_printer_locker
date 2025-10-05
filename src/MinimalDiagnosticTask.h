/*
 * Minimal Diagnostic Task Implementation
 * This is a working example of how to implement FsmOS tasks
 */

#include <Arduino.h>
#include <FsmOS.h>

class MinimalDiagnosticTask : public Task
{
public:
    void on_start() override
    {
        Serial.println(F("MinimalDiagnosticTask: on_start() called"));
    }

    void step() override
    {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 2000)
        {
            Serial.println(F("MinimalDiagnosticTask: step() running"));
            lastPrint = millis();
        }
    }
};
