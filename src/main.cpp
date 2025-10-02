#include <Arduino.h>
#include <FsmOS.h>
#include "YellowButtonTask.h"
#include "KeypadTask.h"
#include "StatusLEDTask.h"
#include "LightTask.h"
#include "PasswordManagerTask.h"
#include "DoorControlTask.h"
#include "DoorSensorTask.h"
#include "EventHandlerTask.h"
#include "BuzzerTask.h"
#include "ChildLockTask.h"
#include "DiagnosticTask.h"
#include "SerialCommandTask.h"
#include "MBLightSensorTask.h"
#include "DeviceRunningSensorTask.h"

// Create task instances
YellowButtonTask yellowButtonTask;
KeypadTask keypadTask;
StatusLEDTask statusLEDTask;
LightTask lightTask;
PasswordManagerTask passwordManagerTask;
DoorControlTask doorControlTask;
DoorSensorTask doorSensorTask;
EventHandlerTask eventHandlerTask;
BuzzerTask buzzerTask;
ChildLockTask childLockTask;
DiagnosticTask diagnosticTask;
SerialCommandTask serialCommandTask;
MBLightSensorTask mbLightSensorTask;
DeviceRunningSensorTask deviceRunningSensorTask;

void setup() {
    Serial.begin(9600);
    OS.begin_with_logger();
    // Use logger helpers via tasks elsewhere; here we use OS directly for boot messages
    OS.logMessage(nullptr, LOG_INFO, F("3D Printer Locker System Starting"));
    
    // Print reset info early to diagnose unexpected resets
    {
        ResetInfo resetInfo;
        if (OS.get_reset_info(resetInfo)) {
            OS.logFormatted(nullptr, LOG_INFO, F("Reset info: reason=%u lastTask=%u"), resetInfo.reset_reason, resetInfo.last_task_id);
        }
    }
    
    // Enable watchdog timer with 2-second timeout
    OS.enable_watchdog(WDTO_2S);
    OS.logMessage(nullptr, LOG_INFO, F("Watchdog enabled (2s timeout)"));
    
    // Set task names
    yellowButtonTask.set_name(F("YellowButton"));
    keypadTask.set_name(F("Keypad"));
    statusLEDTask.set_name(F("StatusLED"));
    lightTask.set_name(F("Light"));
    passwordManagerTask.set_name(F("PasswordMgr"));
    doorControlTask.set_name(F("DoorControl"));
    doorSensorTask.set_name(F("DoorSensor"));
    eventHandlerTask.set_name(F("EventHandler"));
    buzzerTask.set_name(F("Buzzer"));
    childLockTask.set_name(F("ChildLock"));
    diagnosticTask.set_name(F("Diagnostic"));
    serialCommandTask.set_name(F("SerialCmd"));
    mbLightSensorTask.set_name(F("MBLightSensor"));
    deviceRunningSensorTask.set_name(F("DeviceRunning"));
    
    // Add tasks
    OS.add(&yellowButtonTask);
    OS.add(&keypadTask);
    OS.add(&statusLEDTask);
    OS.add(&lightTask);
    OS.add(&passwordManagerTask);
    OS.add(&doorControlTask);
    OS.add(&doorSensorTask);
    OS.add(&eventHandlerTask);
    OS.add(&buzzerTask);
    OS.add(&childLockTask);
    OS.add(&diagnosticTask);
    OS.add(&serialCommandTask);
    OS.add(&mbLightSensorTask);
    OS.add(&deviceRunningSensorTask);
    
    OS.logMessage(nullptr, LOG_INFO, F("System initialized and ready"));
}

void loop() {
    // Run the FSM OS
    OS.loop_once();
}