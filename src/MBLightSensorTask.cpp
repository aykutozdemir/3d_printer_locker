#include "MBLightSensorTask.h"

void MBLightSensorTask::on_start()
{
    pinMode(MB_LIGHT_SENSOR_PIN, INPUT_PULLUP);

    logInfo(F("MBLightSensor started"));

    // Read initial state
    lastSensorState = digitalRead(MB_LIGHT_SENSOR_PIN);
    logInfof(F("Init state = %S"), lastSensorState ? F("HIGH") : F("LOW"));
}

void MBLightSensorTask::step()
{
    readMBLightSensor();

    // Process any received messages
}

void MBLightSensorTask::on_msg(const MsgData &msg)
{
    // This task doesn't handle incoming messages
    (void)msg;
}

void MBLightSensorTask::readMBLightSensor()
{
    bool currentState = digitalRead(MB_LIGHT_SENSOR_PIN);

    // Check for state change (no debouncing needed for motherboard signal)
    if (currentState != lastSensorState)
    {
        lastSensorState = currentState;

        // Publish the state change with inverted logic
        // HIGH = light should be OFF, LOW = light should be ON
        uint8_t lightCommand = currentState ? 0 : 1;
        publish(TOPIC_MB_LIGHT_SENSOR_EVENTS, EVT_MB_LIGHT_SENSOR_CHANGED, lightCommand);

        logInfof(F("State = %S"), currentState ? F("HIGH") : F("LOW"));
    }
}
