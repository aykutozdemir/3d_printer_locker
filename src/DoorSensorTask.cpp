#include "DoorSensorTask.h"

DoorSensorTask::DoorSensorTask() : Task(F("DoorSensor"))
{
    setPeriod(50); // Check every 50ms for responsive door detection
    frontDoorState = false;
    topDoorState = false;
    lastFrontDoorState = false;
    lastTopDoorState = false;
    lastDebounceTime = 0;
}

void DoorSensorTask::on_start()
{
    // Initialize door sensor pins as inputs with pullup
    pinMode(FRONT_DOOR_SENSOR_PIN, INPUT_PULLUP);
    pinMode(TOP_DOOR_SENSOR_PIN, INPUT_PULLUP);

    // Read initial states
    readDoorSensors();
    lastFrontDoorState = frontDoorState;
    lastTopDoorState = topDoorState;

    logInfo(F("Started - Front=D8, Top=D9"));
    logInfo(F("Front=GND closed, Top=GND opened"));
}

void DoorSensorTask::on_msg(const MsgData &msg)
{
    // Door sensor task doesn't need to handle incoming messages
    // It only publishes door state changes
}

void DoorSensorTask::step()
{
    readDoorSensors();

    // Check for state changes and publish events
    if (frontDoorState != lastFrontDoorState)
    {
        lastFrontDoorState = frontDoorState;

        if (frontDoorState)
        {
            // Front door opened (sensor reads LOW/GND)
            publish(TOPIC_DOOR_SENSOR_EVENTS, EVT_DOOR_FRONT_OPENED, 0);
            logInfo(F("Front opened"));
        }
        else
        {
            // Front door closed (sensor reads HIGH)
            publish(TOPIC_DOOR_SENSOR_EVENTS, EVT_DOOR_FRONT_CLOSED, 0);
            logInfo(F("Front closed"));
        }
    }

    if (topDoorState != lastTopDoorState)
    {
        lastTopDoorState = topDoorState;

        if (topDoorState)
        {
            // Top door opened (sensor reads LOW/GND when closed, so HIGH when opened)
            publish(TOPIC_DOOR_SENSOR_EVENTS, EVT_DOOR_TOP_OPENED, 0);
            logInfo(F("Top opened"));
        }
        else
        {
            // Top door closed (sensor reads LOW/GND)
            publish(TOPIC_DOOR_SENSOR_EVENTS, EVT_DOOR_TOP_CLOSED, 0);
            logInfo(F("Top closed"));
        }
    }
}

void DoorSensorTask::readDoorSensors()
{
    // Front door: GND when closed, HIGH when opened
    // frontDoorState = true means door is opened
    frontDoorState = (digitalRead(FRONT_DOOR_SENSOR_PIN) == HIGH);

    // Top door: GND when opened, HIGH when closed
    // topDoorState = true means door is opened (inverted from sensor)
    topDoorState = (digitalRead(TOP_DOOR_SENSOR_PIN) == LOW);
}
