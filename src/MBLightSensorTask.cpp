#include "MBLightSensorTask.h"

void MBLightSensorTask::on_start() {
    pinMode(MB_LIGHT_SENSOR_PIN, INPUT_PULLUP);
    
    log_info(F("Task started - Pin=D7"));
    log_info(F("Monitors motherboard light sensor state"));
    
    // Read initial state
    lastSensorState = digitalRead(MB_LIGHT_SENSOR_PIN);
    {
        const __FlashStringHelper* s = lastSensorState ? F("HIGH") : F("LOW");
        log_infof(F("Initial state = %s"), s);
    }
}

void MBLightSensorTask::step() {
    readMBLightSensor();
    
    // Process any received messages
}

void MBLightSensorTask::on_msg(const MsgData& msg) {
    // This task doesn't handle incoming messages
    (void)msg;
}

void MBLightSensorTask::readMBLightSensor() {
    bool currentState = digitalRead(MB_LIGHT_SENSOR_PIN);
    
    // Check for state change (no debouncing needed for motherboard signal)
    if (currentState != lastSensorState) {
        lastSensorState = currentState;
        
        // Publish the state change with inverted logic
        // HIGH = light should be OFF, LOW = light should be ON
        publish(TOPIC_MB_LIGHT_SENSOR_EVENTS, EVT_MB_LIGHT_SENSOR_CHANGED, currentState ? 0 : 1, nullptr);
        
        {
            const __FlashStringHelper* s = currentState ? F("HIGH") : F("LOW");
            log_infof(F("State changed to %s"), s);
        }
    }
}
