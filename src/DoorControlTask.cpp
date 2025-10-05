#include "DoorControlTask.h"

DoorControlTask::DoorControlTask() : Task(F("DoorControl"))
{
    setPeriod(100); // Check every 100ms
    frontDoorReleased = false;
    topDoorReleased = false;
    frontDoorOpened = false;
    topDoorOpened = false;
    waitingForDoorOpen = false;
    lastLEDState = false; // false = locked, true = unlocked
    frontDoorNeedsReengage = false;
    topDoorNeedsReengage = false;
    unauthorizedAccessActive = false;
}

void DoorControlTask::on_start()
{
    // Initialize door control pins as outputs
    pinMode(FRONT_DOOR_PIN, OUTPUT);
    pinMode(TOP_DOOR_PIN, OUTPUT);

    // Subscribe to door events and door sensor events
    subscribe(TOPIC_DOOR_EVENTS);
    subscribe(TOPIC_DOOR_SENSOR_EVENTS);

    // Start with all doors locked (magnets engaged)
    lockAllDoors();

    logInfo(F("DoorCtl start"));
    logInfo(F("All locked"));
}

void DoorControlTask::on_msg(const MsgData &msg)
{
    switch (msg.type)
    {
        case EVT_DOOR_TOP_RELEASE:
            releaseTopDoor();
            break;

        case EVT_DOOR_FRONT_RELEASE:
            releaseFrontDoor();
            break;

        case EVT_DOOR_BOTH_RELEASE:
            releaseBothDoors();
            break;

        // Handle door sensor events
        case EVT_DOOR_FRONT_OPENED:
        case EVT_DOOR_TOP_OPENED:
        case EVT_DOOR_FRONT_CLOSED:
        case EVT_DOOR_TOP_CLOSED:
            handleDoorSensorEvent(msg.type);
            break;

        default:
            break;
    }
}

void DoorControlTask::step()
{
    // Check if front door delay has passed - now re-engage magnet (turn back to LOW)
    if (frontDoorOpened && frontDoorReleased && frontDoorOpenTimer.isExpired())
    {
        digitalWrite(FRONT_DOOR_PIN, LOW); // Re-engage magnet after 1.5s delay
        logInfo(F("Front re-engage"));
    }

    // Check if top door delay has passed - now re-engage magnet (turn back to LOW)
    if (topDoorOpened && topDoorReleased && topDoorOpenTimer.isExpired())
    {
        digitalWrite(TOP_DOOR_PIN, LOW); // Re-engage magnet after 1.5s delay
        logInfo(F("Top re-engage"));
    }

    // Check for magnet re-engagement delays
    if (frontDoorNeedsReengage && frontDoorCloseTimer.isExpired())
    {
        digitalWrite(FRONT_DOOR_PIN, LOW); // Re-engage magnet
        frontDoorNeedsReengage = false;
        logInfo(F("Front re-engage"));
    }

    if (topDoorNeedsReengage && topDoorCloseTimer.isExpired())
    {
        digitalWrite(TOP_DOOR_PIN, LOW); // Re-engage magnet
        topDoorNeedsReengage = false;
        logInfo(F("Top re-engage"));
    }

    // Update status LEDs based on door state
    updateStatusLEDs();

    // Process any received messages
}

void DoorControlTask::releaseFrontDoor()
{
    digitalWrite(FRONT_DOOR_PIN, HIGH); // Immediately turn off magnet (unlock door)
    frontDoorReleased = true;
    waitingForDoorOpen = true;

    // Set LED to "to be opened" state (green blinking)
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);

    // Play door released sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    logInfo(F("Front released"));
}

void DoorControlTask::releaseTopDoor()
{
    digitalWrite(TOP_DOOR_PIN, HIGH); // Immediately turn off magnet (unlock door)
    topDoorReleased = true;
    waitingForDoorOpen = true;

    // Set LED to "to be opened" state (green blinking)
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);

    // Play door released sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    logInfo(F("Top released"));
}

void DoorControlTask::releaseBothDoors()
{
    digitalWrite(FRONT_DOOR_PIN, HIGH); // Immediately turn off magnet (unlock door)
    digitalWrite(TOP_DOOR_PIN, HIGH);   // Immediately turn off magnet (unlock door)
    frontDoorReleased = true;
    topDoorReleased = true;
    waitingForDoorOpen = true;

    // Set LED to "to be opened" state (green blinking)
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);

    // Play door released sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    logInfo(F("Both released"));
}

void DoorControlTask::lockAllDoors()
{
    digitalWrite(FRONT_DOOR_PIN, LOW); // Power off = magnet engaged (locked)
    digitalWrite(TOP_DOOR_PIN, LOW);   // Power off = magnet engaged (locked)
    frontDoorReleased = false;
    topDoorReleased = false;
    waitingForDoorOpen = false;

    // Play door closed sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_CLOSED, 0, nullptr);

    logInfo(F("All locked"));
}

void DoorControlTask::updateStatusLEDs()
{
    // Check if door state has changed
    bool currentLEDState = (frontDoorOpened || topDoorOpened);

    if (currentLEDState != lastLEDState)
    {
        lastLEDState = currentLEDState;

        if (currentLEDState)
        {
            // At least one door is actually opened - set status LED to green (unlocked)
            publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_UNLOCKED, 0, nullptr);
        }
        else
        {
            // All doors are closed - set status LED to red (locked)
            publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0, nullptr);
        }
    }
}

void DoorControlTask::handleDoorSensorEvent(uint8_t eventType)
{
    switch (eventType)
    {
        case EVT_DOOR_FRONT_OPENED:
            frontDoorOpened = true;
            frontDoorOpenTimer = createTimerTyped<Timer16>(MAGNET_DELAY_MS); // Start timer for re-engagement
            if (frontDoorReleased)
            {
                logInfo(F("Front opened"));
            }
            else
            {
                // Unauthorized access - door opened without password
                logWarn(F("UNAUTH Front!"));
                unauthorizedAccessActive = true;
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_START, 0, nullptr);
            }
            break;

        case EVT_DOOR_TOP_OPENED:
            topDoorOpened = true;
            topDoorOpenTimer = createTimerTyped<Timer16>(MAGNET_DELAY_MS); // Start timer for re-engagement
            if (topDoorReleased)
            {
                logInfo(F("Top opened"));
            }
            else
            {
                // Unauthorized access - door opened without password
                logWarn(F("UNAUTH Top!"));
                unauthorizedAccessActive = true;
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_START, 0, nullptr);
            }
            break;

        case EVT_DOOR_FRONT_CLOSED:
            frontDoorOpened = false;
            // Stop angry sound when door is closed (only if unauthorized access was active)
            if (unauthorizedAccessActive)
            {
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_STOP, 0, nullptr);
                unauthorizedAccessActive = false;
                logInfo(F("Front closed"));
            }
            if (frontDoorReleased)
            {
                // Front door is closed - schedule magnet re-engagement
                frontDoorCloseTimer = createTimerTyped<Timer8>(REENGAGE_DELAY_MS); // Start timer for re-engagement
                frontDoorNeedsReengage = true; // Flag to re-engage magnet
                logInfo(F("Front closed"));
            }
            break;

        case EVT_DOOR_TOP_CLOSED:
            topDoorOpened = false;
            // Stop angry sound when door is closed (only if unauthorized access was active)
            if (unauthorizedAccessActive)
            {
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_STOP, 0, nullptr);
                unauthorizedAccessActive = false;
                logInfo(F("Top closed"));
            }
            if (topDoorReleased)
            {
                // Top door is closed - schedule magnet re-engagement
                topDoorCloseTimer = createTimerTyped<Timer8>(REENGAGE_DELAY_MS); // Start timer for re-engagement
                topDoorNeedsReengage = true; // Flag to re-engage magnet
                logInfo(F("Top door closed"));
            }
            break;
    }

    // Check if all doors are closed and should be locked
    if (!frontDoorOpened && !topDoorOpened && (frontDoorReleased || topDoorReleased))
    {
        // All doors are closed - lock all doors
        lockAllDoors();
        logInfo(F("All closed"));
    }

    // Update LED state based on door status
    if (waitingForDoorOpen && (frontDoorOpened || topDoorOpened))
    {
        // Door has been opened - change to green solid (unlocked)
        waitingForDoorOpen = false;
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_UNLOCKED, 0, nullptr);
        logInfo(F("Door opened -> LED green"));
    }
}
