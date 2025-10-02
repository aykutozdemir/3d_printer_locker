#include "DeviceRunningSensorTask.h"

void DeviceRunningSensorTask::on_start() {
    pinMode(DEVICE_RUNNING_SENSOR_PIN, INPUT_PULLUP);
    
    log_info(F("Task started - Pin=A4"));
    log_info(F("Monitors device running state"));
    
    // Read initial state
    lastDeviceRunningState = digitalRead(DEVICE_RUNNING_SENSOR_PIN);
    const __FlashStringHelper* stateStr0 = lastDeviceRunningState ? F("RUNNING") : F("STOPPED");
    log_infof(F("Initial state = %s"), stateStr0);
}

void DeviceRunningSensorTask::step() {
    readDeviceRunningSensor();
    
    // Process any received messages
}

void DeviceRunningSensorTask::on_msg(const MsgData& msg) {
    // This task doesn't handle incoming messages
    (void)msg;
}

void DeviceRunningSensorTask::readDeviceRunningSensor() {
    bool currentState = digitalRead(DEVICE_RUNNING_SENSOR_PIN);
    
    // Check for state change
    if (currentState != lastDeviceRunningState) {
        lastDeviceRunningState = currentState;
        
        // Publish the state change
        publish(TOPIC_DEVICE_RUNNING_EVENTS, EVT_DEVICE_RUNNING_CHANGED, currentState ? 1 : 0, nullptr);
        
        const __FlashStringHelper* stateStr1 = currentState ? F("RUNNING") : F("STOPPED");
        log_infof(F("State changed to %s"), stateStr1);
    }
}
