/*
 * 3D Printer Locker System - Complete Implementation
 * This version includes all task components for the locker system
 */

#include <Arduino.h>
#include <FsmOS.h>
#include <avr/wdt.h>

// Include all task headers
#include "BuzzerTask.h"
#include "ChildLockTask.h"
#include "DeviceRunningSensorTask.h"
#include "DiagnosticTask.h"
#include "DoorControlTask.h"
#include "DoorSensorTask.h"
#include "EventHandlerTask.h"
#include "KeypadTask.h"
#include "LightTask.h"
#include "MBLightSensorTask.h"
#include "PasswordManagerTask.h"
#include "SerialCommandTask.h"
#include "StatusLEDTask.h"
#include "YellowButtonTask.h"

// Static task instances (no dynamic allocation)
BuzzerTask buzzerTask;
ChildLockTask childLockTask;
DeviceRunningSensorTask deviceRunningSensorTask;
DiagnosticTask diagnosticTask;
DoorControlTask doorControlTask;
DoorSensorTask doorSensorTask;
EventHandlerTask eventHandlerTask;
KeypadTask keypadTask;
LightTask lightTask;
MBLightSensorTask mbLightSensorTask;
PasswordManagerTask passwordManagerTask;
SerialCommandTask serialCommandTask;
StatusLEDTask statusLEDTask;
YellowButtonTask yellowButtonTask;

// System state variables
bool systemInitialized = false;
unsigned long systemStartTime = 0;

// OS Timers
Timer16 ledToggleTimer;
Timer16 debugMessageTimer;

/**
 * Initialize all task instances
 */
void initializeAllTasks()
{
    Serial.println(F("Init"));
    Serial.println(F("Static alloc"));

    // Task instances are already created as static objects
    Serial.println(F("Ready"));
}

/**
 * Configure complete scheduler with all tasks
 */
void initializeCompleteScheduler()
{
    Serial.println(F("Config OS"));

    // Set task periods (in milliseconds)
    buzzerTask.setPeriod(50);                       // 50ms - High frequency for sound control
    childLockTask.setPeriod(100);                    // 100ms - Child lock management
    deviceRunningSensorTask.setPeriod(200);          // 200ms - Device sensor monitoring
    diagnosticTask.setPeriod(1000);                  // 1000ms - Diagnostic checks
    doorControlTask.setPeriod(100);                  // 100ms - Door control
    doorSensorTask.setPeriod(50);                    // 50ms - Door sensor monitoring
    eventHandlerTask.setPeriod(100);                 // 100ms - Event handling
    keypadTask.setPeriod(10);                        // 10ms - High frequency keypad scanning
    lightTask.setPeriod(50);                         // 50ms - Light control
    mbLightSensorTask.setPeriod(200);                // 200ms - Motherboard light sensor
    passwordManagerTask.setPeriod(100);              // 100ms - Password management
    serialCommandTask.setPeriod(50);                 // 50ms - Serial command processing
    statusLEDTask.setPeriod(50);                     // 50ms - Status LED control
    yellowButtonTask.setPeriod(10);                  // 10ms - High frequency button scanning

    // Add tasks to scheduler - ALL TASKS ACTIVE
    Serial.println(F("Adding tasks..."));
    bool success = true;
    success &= OS.add(&buzzerTask); Serial.print(F("Buzzer: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&childLockTask); Serial.print(F("ChildLock: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&deviceRunningSensorTask); Serial.print(F("DeviceRunning: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&diagnosticTask); Serial.print(F("Diagnostic: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&doorControlTask); Serial.print(F("DoorControl: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&doorSensorTask); Serial.print(F("DoorSensor: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&eventHandlerTask); Serial.print(F("EventHandler: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&keypadTask); Serial.print(F("Keypad: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&lightTask); Serial.print(F("Light: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&mbLightSensorTask); Serial.print(F("MBLightSensor: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&passwordManagerTask); Serial.print(F("PasswordManager: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&serialCommandTask); Serial.print(F("SerialCommand: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&statusLEDTask); Serial.print(F("StatusLED: ")); Serial.println(success ? F("OK") : F("FAIL"));
    success &= OS.add(&yellowButtonTask); Serial.print(F("YellowButton: ")); Serial.println(success ? F("OK") : F("FAIL"));

    Serial.print(F("All tasks added: "));
    Serial.println(success ? F("SUCCESS") : F("SOME FAILED"));

    Serial.println(F("Scheduler OK"));
    Serial.print(F("Tasks: "));
    Serial.println(OS.getTaskCount());
}

/**
 * Display system information
 */
void displaySystemInfo()
{
    Serial.println();
    Serial.println(F("=== 3D PRINTER LOCKER ==="));
    Serial.println(F("HW: Nano"));
    Serial.println(F("FW: FsmOS v1.3.0"));
    Serial.print(F("Build: "));
    Serial.print(F(__DATE__));
    Serial.print(F(" "));
    Serial.println(F(__TIME__));

    // Display reset reason
    Serial.println(F("=== Reset ==="));
    ResetInfo resetInfo;
    if (OS.getResetInfo(resetInfo))
    {
        Serial.print(F("  Reason: "));
        Serial.println(resetInfo.resetReason);
        Serial.print(F("  LastTask: "));
        Serial.println(resetInfo.lastTaskId);
    }
    else
    {
        Serial.println(F("  No reset info"));
    }

    // Display reset cause flags
    uint8_t resetFlags = OS.getResetCauseFlags();
    Serial.print(F("  Flags: 0x"));
    Serial.println(resetFlags, HEX);

    // Display reset cause
    ResetCause resetCause = OS.getResetCause();
    Serial.print(F("  Cause: "));
    switch (resetCause)
    {
        case RESET_UNKNOWN:
            Serial.println(F("UNK"));
            break;
        case RESET_POWER_ON:
            Serial.println(F("POR"));
            break;
        case RESET_EXTERNAL:
            Serial.println(F("EXT"));
            break;
        case RESET_BROWN_OUT:
            Serial.println(F("BOR"));
            break;
        case RESET_WATCHDOG:
            Serial.println(F("WDT"));
            break;
        case RESET_MULTIPLE:
            Serial.println(F("MULT"));
            break;
        default:
            Serial.println(F("Other"));
            break;
    }
}

/**
 * Main setup function
 */
void setup()
{
    // Disable watchdog timer during initialization
    wdt_disable();

    // Initialize serial communication
    Serial.begin(9600);

    // Initialize built-in LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Initialize OS timers
    ledToggleTimer.startTimer(1000);        // LED toggle every 1 second
    debugMessageTimer.startTimer(10000);   // Debug message every 10 seconds

    // Wait for serial to initialize
    while (!Serial && millis() < 3000)
    {
        delay(10);
    }

    // Record system start time
    systemStartTime = millis();

    // Display system information
    displaySystemInfo();

    // Initialize all task instances
    initializeAllTasks();

    // Configure complete scheduler
    initializeCompleteScheduler();

    // Start the scheduler
    Serial.println(F("Starting OS..."));
    Serial.println(F("OS.begin()..."));

    OS.begin();

    // Enable watchdog timer with 2 seconds timeout AFTER OS.begin()
    OS.enableWatchdog(WDTO_2S);  // AVR: WDTO_2S = 2 seconds
    Serial.println(F("WDT=2s"));

    Serial.println(F("OS OK"));

    // Mark system as initialized
    systemInitialized = true;

    Serial.println(F("Init done"));
    Serial.println(F("System ready"));
    Serial.println(F("Use CLI"));
    Serial.println();
}

/**
 * Main loop function
 */
void loop()
{
    // Debug message using OS timer
    if (debugMessageTimer.isExpired())
    {
        Serial.println(F("LOOP OK"));
        debugMessageTimer.startTimer(10000); // Restart timer for next message
    }

    // Call FsmOS scheduler to execute tasks
    if (systemInitialized)
    {
        OS.loopOnce();  // Use loopOnce instead of step
    }

    // Simple LED toggle using OS timer
    if (ledToggleTimer.isExpired())
    {
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
        ledToggleTimer.startTimer(1000); // Restart timer for next toggle
    }

    // Small delay
    delay(1);
}
