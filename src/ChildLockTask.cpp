#include "ChildLockTask.h"

ChildLockTask::ChildLockTask() : Task(F("ChildLock"))
{
    setPeriod(100); // Check every 100ms
    childLockEngaged = true; // Start with child lock engaged (locked)
    deviceRunning = true; // Assume device is running initially
}

void ChildLockTask::on_start()
{
    // Initialize child lock pins as outputs
    pinMode(CHILD_LOCK_POWER_PIN, OUTPUT);
    pinMode(CHILD_LOCK_SCREEN_PIN, OUTPUT);

    // Subscribe to child lock events and device running events
    subscribe(TOPIC_CHILD_LOCK_EVENTS);
    subscribe(TOPIC_DEVICE_RUNNING_EVENTS);
    subscribe(TOPIC_KEYPAD_EVENTS); // listen key events for special functions

    // Start with child lock engaged (screen and power button locked)
    engageChildLock();

    logInfo(F("ChildLock start"));
    logInfo(F("ENGAGED"));
}

void ChildLockTask::on_msg(const MsgData &msg)
{
    switch (msg.type)
    {
        case EVT_CHILD_LOCK_RELEASE:
            releaseChildLock();
            break;
        case EVT_CHILD_LOCK_ENGAGE:
            engageChildLock();
            break;
        case EVT_CHILD_LOCK_TIMEOUT_RESET:
            if (!childLockEngaged)
            {
                childLockTimeoutTimer = createTimerTyped<Timer16>(CHILD_LOCK_TIMEOUT_MS);
                logInfo(F("Timeout reset"));
            }
            break;

        case EVT_DEVICE_RUNNING_CHANGED:
            deviceRunning = (msg.arg == 1);
            logInfo(F("DeviceState changed"));
            updateChildLockState();
            break;
        // Keypad special functions when child lock is disabled
        case EVT_KEYPAD_1_LONG_PRESSED:
            if (!childLockEngaged)
            {
                logInfo(F("Key1 long -> engage"));
                engageChildLock();
            }
            break;
        case EVT_KEYPAD_4_PRESSED:
            if (!childLockEngaged)
            {
                logInfo(F("Key4 -> timeout reset"));
                childLockTimeoutTimer = createTimerTyped<Timer16>(CHILD_LOCK_TIMEOUT_MS);
            }
            break;

        default:
            break;
    }
}

void ChildLockTask::step()
{
    // Process any received messages

    // Auto re-engage child lock after timeout if released
    if (!childLockEngaged && childLockTimeoutTimer.isExpired())
    {
        logInfo(F("Timeout -> engage"));
        engageChildLock();
    }
}

void ChildLockTask::releaseChildLock()
{
    childLockEngaged = false;
    digitalWrite(CHILD_LOCK_POWER_PIN, HIGH); // HIGH = power button unlocked
    digitalWrite(CHILD_LOCK_SCREEN_PIN, HIGH); // HIGH = touchscreen unlocked

    logInfo(F("Released"));

    // Play buzzer sound for child lock release
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_CHILD_LOCK_SELECTED, 0, nullptr);
    // Update status LED to indicate child lock disabled
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_CHILD_UNLOCKED, 0, nullptr);
    // Start timeout countdown
    childLockTimeoutTimer = createTimerTyped<Timer16>(CHILD_LOCK_TIMEOUT_MS);
}

void ChildLockTask::engageChildLock()
{
    childLockEngaged = true;
    updateChildLockState();
    // Timer will be automatically cleared when not in use
    // Set LED back to locked state
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0, nullptr);
}

void ChildLockTask::updateChildLockState()
{
    if (childLockEngaged)
    {
        // Child lock is engaged - control based on device running state
        if (deviceRunning)
        {
            // Device is running - lock both screen and power button
            digitalWrite(CHILD_LOCK_POWER_PIN, LOW); // LOW = power button locked
            digitalWrite(CHILD_LOCK_SCREEN_PIN, LOW); // LOW = touchscreen locked
            logInfo(F("Engaged: running"));
        }
        else
        {
            // Device is stopped - enable power button only (not touchscreen)
            digitalWrite(CHILD_LOCK_POWER_PIN, HIGH); // HIGH = power button unlocked
            digitalWrite(CHILD_LOCK_SCREEN_PIN, LOW); // LOW = touchscreen locked
            logInfo(F("Engaged: stopped"));
        }
    }
    else
    {
        // Child lock is released - enable both
        digitalWrite(CHILD_LOCK_POWER_PIN, HIGH); // HIGH = power button unlocked
        digitalWrite(CHILD_LOCK_SCREEN_PIN, HIGH); // HIGH = touchscreen unlocked
        logInfo(F("Released"));
    }
}
