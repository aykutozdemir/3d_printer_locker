#include "DeviceRunningSensorTask.h"

void DeviceRunningSensorTask::on_start()
{
    pinMode(DEVICE_RUNNING_SENSOR_PIN, INPUT_PULLUP);

    logInfo(F("Started"));

    // Read initial state
    lastDeviceRunningState = digitalRead(DEVICE_RUNNING_SENSOR_PIN);
    //logInfof(F("Initial state = %s"), lastDeviceRunningState ? F("RUNNING") : F("STOPPED"));
}

void DeviceRunningSensorTask::step()
{
    readDeviceRunningSensor();

    // Process any received messages
}

void DeviceRunningSensorTask::on_msg(const MsgData &msg)
{
    // This task doesn't handle incoming messages
    (void)msg;
}

void DeviceRunningSensorTask::readDeviceRunningSensor()
{
    bool currentState = digitalRead(DEVICE_RUNNING_SENSOR_PIN);

    // Check for state change
    if (currentState != lastDeviceRunningState)
    {
        lastDeviceRunningState = currentState;

        // Publish the state change
        publish(TOPIC_DEVICE_RUNNING_EVENTS, EVT_DEVICE_RUNNING_CHANGED, currentState ? 1 : 0);

        logInfof(F("State = %S"), currentState ? F("RUNNING") : F("STOPPED"));
    }
}
