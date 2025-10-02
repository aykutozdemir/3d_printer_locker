#include "ChildLockTask.h"

ChildLockTask::ChildLockTask() : Task(nullptr) {
    set_period(100); // Check every 100ms
    childLockEngaged = true; // Start with child lock engaged (locked)
    deviceRunning = true; // Assume device is running initially
    childLockReleaseTime = 0;
}

void ChildLockTask::on_start() {
    // Initialize child lock pins as outputs
    pinMode(CHILD_LOCK_POWER_PIN, OUTPUT);
    pinMode(CHILD_LOCK_SCREEN_PIN, OUTPUT);
    
    // Subscribe to child lock events and device running events
    subscribe(TOPIC_CHILD_LOCK_EVENTS);
    subscribe(TOPIC_DEVICE_RUNNING_EVENTS);
    subscribe(TOPIC_KEYPAD_EVENTS); // listen key events for special functions
    
    // Start with child lock engaged (screen and power button locked)
    engageChildLock();
    
    log_info(F("Task started - Power=D13, Screen=A0"));
    log_info(F("Initial state - ENGAGED (screen/power locked)"));
}

void ChildLockTask::on_msg(const MsgData& msg) {
    switch (msg.type) {
        case EVT_CHILD_LOCK_RELEASE:
            releaseChildLock();
            break;
        case EVT_CHILD_LOCK_ENGAGE:
            engageChildLock();
            break;
        case EVT_CHILD_LOCK_TIMEOUT_RESET:
            if (!childLockEngaged) {
                childLockReleaseTime = OS.now();
                log_info(F("Child lock timeout reset (1 minute)"));
            }
            break;
            
        case EVT_DEVICE_RUNNING_CHANGED:
            deviceRunning = (msg.arg == 1);
            log_info(F("Device running state changed"));
            updateChildLockState();
            break;
        // Keypad special functions when child lock is disabled
        case EVT_KEYPAD_1_LONG_PRESSED:
            if (!childLockEngaged) {
                log_info(F("Keypad 1 long pressed - engaging child lock immediately"));
                engageChildLock();
            }
            break;
        case EVT_KEYPAD_4_PRESSED:
            if (!childLockEngaged) {
                log_info(F("Keypad 4 pressed - resetting child lock timeout"));
                childLockReleaseTime = OS.now();
            }
            break;
            
        default:
            break;
    }
}

void ChildLockTask::step() {
    // Process any received messages
    
    // Auto re-engage child lock after timeout if released
    if (!childLockEngaged && childLockReleaseTime != 0) {
        if (OS.now() - childLockReleaseTime >= CHILD_LOCK_TIMEOUT_MS) {
            log_info(F("Child lock timeout reached - engaging child lock"));
            engageChildLock();
        }
    }
}

void ChildLockTask::releaseChildLock() {
    childLockEngaged = false;
    digitalWrite(CHILD_LOCK_POWER_PIN, HIGH); // HIGH = power button unlocked
    digitalWrite(CHILD_LOCK_SCREEN_PIN, HIGH); // HIGH = touchscreen unlocked
    
    log_info(F("Released - screen and power button now enabled"));
    
    // Play buzzer sound for child lock release
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_CHILD_LOCK_SELECTED, 0, nullptr);
    // Update status LED to indicate child lock disabled
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_CHILD_UNLOCKED, 0, nullptr);
    // Start timeout countdown
    childLockReleaseTime = OS.now();
}

void ChildLockTask::engageChildLock() {
    childLockEngaged = true;
    updateChildLockState();
    childLockReleaseTime = 0; // clear timeout tracking
    // Set LED back to locked state
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0, nullptr);
}

void ChildLockTask::updateChildLockState() {
    if (childLockEngaged) {
        // Child lock is engaged - control based on device running state
        if (deviceRunning) {
            // Device is running - lock both screen and power button
            digitalWrite(CHILD_LOCK_POWER_PIN, LOW); // LOW = power button locked
            digitalWrite(CHILD_LOCK_SCREEN_PIN, LOW); // LOW = touchscreen locked
            log_info(F("Engaged - screen and power button locked (device running)"));
        } else {
            // Device is stopped - enable power button only (not touchscreen)
            digitalWrite(CHILD_LOCK_POWER_PIN, HIGH); // HIGH = power button unlocked
            digitalWrite(CHILD_LOCK_SCREEN_PIN, LOW); // LOW = touchscreen locked
            log_info(F("Engaged - power button enabled, screen locked (device stopped)"));
        }
    } else {
        // Child lock is released - enable both
        digitalWrite(CHILD_LOCK_POWER_PIN, HIGH); // HIGH = power button unlocked
        digitalWrite(CHILD_LOCK_SCREEN_PIN, HIGH); // HIGH = touchscreen unlocked
        log_info(F("Released - screen and power button enabled"));
    }
}
