#include "DoorControlTask.h"

DoorControlTask::DoorControlTask() {
    set_period(100); // Check every 100ms
    frontDoorReleased = false;
    topDoorReleased = false;
    frontDoorOpened = false;
    topDoorOpened = false;
    waitingForDoorOpen = false;
    lastLEDState = false; // false = locked, true = unlocked
    frontDoorOpenTime = 0;
    topDoorOpenTime = 0;
    frontDoorCloseTime = 0;
    topDoorCloseTime = 0;
    frontDoorNeedsReengage = false;
    topDoorNeedsReengage = false;
    unauthorizedAccessActive = false;
}

void DoorControlTask::on_start() {
    // Initialize door control pins as outputs
    pinMode(FRONT_DOOR_PIN, OUTPUT);
    pinMode(TOP_DOOR_PIN, OUTPUT);
    
    // Subscribe to door events and door sensor events
    subscribe(TOPIC_DOOR_EVENTS);
    subscribe(TOPIC_DOOR_SENSOR_EVENTS);
    
    // Start with all doors locked (magnets engaged)
    lockAllDoors();
    
    log_info(F("Task started - Front=D11, Top=D12"));
    log_info(F("All doors locked (magnets engaged)"));
}

void DoorControlTask::on_msg(const MsgData& msg) {
    switch (msg.type) {
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

void DoorControlTask::step() {
    // Check for magnet delay timing
    unsigned long currentTime = OS.now();
    
    // Check if front door delay has passed - now re-engage magnet (turn back to LOW)
    if (frontDoorOpened && frontDoorReleased && frontDoorOpenTime > 0) {
        if (currentTime - frontDoorOpenTime >= MAGNET_DELAY_MS) {
            digitalWrite(FRONT_DOOR_PIN, LOW); // Re-engage magnet after 1.5s delay
            log_info(F("Front door magnet re-engaged after 1.5s delay"));
            frontDoorOpenTime = 0; // Reset to avoid repeated messages
        }
    }
    
    // Check if top door delay has passed - now re-engage magnet (turn back to LOW)
    if (topDoorOpened && topDoorReleased && topDoorOpenTime > 0) {
        if (currentTime - topDoorOpenTime >= MAGNET_DELAY_MS) {
            digitalWrite(TOP_DOOR_PIN, LOW); // Re-engage magnet after 1.5s delay
            log_info(F("Top door magnet re-engaged after 1.5s delay"));
            topDoorOpenTime = 0; // Reset to avoid repeated messages
        }
    }
    
    // Check for magnet re-engagement delays
    if (frontDoorNeedsReengage && frontDoorCloseTime > 0) {
        if (currentTime - frontDoorCloseTime >= REENGAGE_DELAY_MS) {
            digitalWrite(FRONT_DOOR_PIN, LOW); // Re-engage magnet
            frontDoorNeedsReengage = false;
            frontDoorCloseTime = 0;
            log_info(F("Front door magnet re-engaged after delay"));
        }
    }
    
    if (topDoorNeedsReengage && topDoorCloseTime > 0) {
        if (currentTime - topDoorCloseTime >= REENGAGE_DELAY_MS) {
            digitalWrite(TOP_DOOR_PIN, LOW); // Re-engage magnet
            topDoorNeedsReengage = false;
            topDoorCloseTime = 0;
            log_info(F("Top door magnet re-engaged after delay"));
        }
    }
    
    // Update status LEDs based on door state
    updateStatusLEDs();
    
    // Process any received messages
}

void DoorControlTask::releaseFrontDoor() {
    digitalWrite(FRONT_DOOR_PIN, HIGH); // Immediately turn off magnet (unlock door)
    frontDoorReleased = true;
    waitingForDoorOpen = true;
    
    // Set LED to "to be opened" state (green blinking)
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);
    
    // Play door released sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    log_info(F("Front door released (magnet OFF) - waiting for door to open"));
}

void DoorControlTask::releaseTopDoor() {
    digitalWrite(TOP_DOOR_PIN, HIGH); // Immediately turn off magnet (unlock door)
    topDoorReleased = true;
    waitingForDoorOpen = true;
    
    // Set LED to "to be opened" state (green blinking)
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);
    
    // Play door released sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    log_info(F("Top door released (magnet OFF) - waiting for door to open"));
}

void DoorControlTask::releaseBothDoors() {
    digitalWrite(FRONT_DOOR_PIN, HIGH); // Immediately turn off magnet (unlock door)
    digitalWrite(TOP_DOOR_PIN, HIGH);   // Immediately turn off magnet (unlock door)
    frontDoorReleased = true;
    topDoorReleased = true;
    waitingForDoorOpen = true;
    
    // Set LED to "to be opened" state (green blinking)
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);
    
    // Play door released sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    log_info(F("Both doors released (magnets OFF) - waiting for door to open"));
}

void DoorControlTask::lockAllDoors() {
    digitalWrite(FRONT_DOOR_PIN, LOW); // Power off = magnet engaged (locked)
    digitalWrite(TOP_DOOR_PIN, LOW);   // Power off = magnet engaged (locked)
    frontDoorReleased = false;
    topDoorReleased = false;
    waitingForDoorOpen = false;
    
    // Play door closed sound
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_CLOSED, 0, nullptr);
    
    log_info(F("All doors locked (magnets engaged)"));
}

void DoorControlTask::updateStatusLEDs() {
    // Check if door state has changed
    bool currentLEDState = (frontDoorOpened || topDoorOpened);
    
    if (currentLEDState != lastLEDState) {
        lastLEDState = currentLEDState;
        
        if (currentLEDState) {
            // At least one door is actually opened - set status LED to green (unlocked)
            publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_UNLOCKED, 0, nullptr);
        } else {
            // All doors are closed - set status LED to red (locked)
            publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0, nullptr);
        }
    }
}

void DoorControlTask::handleDoorSensorEvent(uint8_t eventType) {
    switch (eventType) {
        case EVT_DOOR_FRONT_OPENED:
            frontDoorOpened = true;
            frontDoorOpenTime = OS.now(); // Record time when door was opened
            if (frontDoorReleased) {
                log_info(F("Front door opened - waiting 1.5s before turning off magnet"));
            } else {
                // Unauthorized access - door opened without password
                log_warn(F("UNAUTHORIZED ACCESS - Front door opened without password!"));
                unauthorizedAccessActive = true;
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_START, 0, nullptr);
            }
            break;
            
        case EVT_DOOR_TOP_OPENED:
            topDoorOpened = true;
            topDoorOpenTime = OS.now(); // Record time when door was opened
            if (topDoorReleased) {
                log_info(F("Top door opened - waiting 1.5s before turning off magnet"));
            } else {
                // Unauthorized access - door opened without password
                log_warn(F("UNAUTHORIZED ACCESS - Top door opened without password!"));
                unauthorizedAccessActive = true;
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_START, 0, nullptr);
            }
            break;
            
        case EVT_DOOR_FRONT_CLOSED:
            frontDoorOpened = false;
            frontDoorOpenTime = 0; // Reset timing
            // Stop angry sound when door is closed (only if unauthorized access was active)
            if (unauthorizedAccessActive) {
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_STOP, 0, nullptr);
                unauthorizedAccessActive = false;
                log_info(F("Front door closed - stopping angry sound"));
            }
            if (frontDoorReleased) {
                // Front door is closed - schedule magnet re-engagement
                frontDoorCloseTime = OS.now(); // Record when door was closed
                frontDoorNeedsReengage = true; // Flag to re-engage magnet
                log_info(F("Front door closed - magnet will re-engage in 100ms"));
            }
            break;
            
        case EVT_DOOR_TOP_CLOSED:
            topDoorOpened = false;
            topDoorOpenTime = 0; // Reset timing
            // Stop angry sound when door is closed (only if unauthorized access was active)
            if (unauthorizedAccessActive) {
                publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND_STOP, 0, nullptr);
                unauthorizedAccessActive = false;
                log_info(F("Top door closed - stopping angry sound"));
            }
            if (topDoorReleased) {
                // Top door is closed - schedule magnet re-engagement
                topDoorCloseTime = OS.now(); // Record when door was closed
                topDoorNeedsReengage = true; // Flag to re-engage magnet
                log_info(F("Top door closed - magnet will re-engage in 100ms"));
            }
            break;
    }
    
    // Check if all doors are closed and should be locked
    if (!frontDoorOpened && !topDoorOpened && (frontDoorReleased || topDoorReleased)) {
        // All doors are closed - lock all doors
        lockAllDoors();
        log_info(F("All doors closed - locking all doors"));
    }
    
    // Update LED state based on door status
    if (waitingForDoorOpen && (frontDoorOpened || topDoorOpened)) {
        // Door has been opened - change to green solid (unlocked)
        waitingForDoorOpen = false;
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_UNLOCKED, 0, nullptr);
        log_info(F("Door opened - LED changed to green solid"));
    }
}
