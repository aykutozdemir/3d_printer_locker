#include "StatusLEDTask.h"

StatusLEDTask::StatusLEDTask() {
    set_period(50); // Update LEDs every 50ms for smooth blinking
    currentState = LED_LOCKED; // Start in locked state
    redLedState = false;
    greenLedState = false;
    lastBlinkTime = 0;
    altToggle = false;
}

void StatusLEDTask::on_start() {
    // Initialize LED pins as outputs
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    
    // Subscribe to LED events
    subscribe(TOPIC_STATUS_LED_EVENTS);
    
    // Start with locked state (red solid)
    setLEDs(true, false);
    currentState = LED_LOCKED;
    
    log_info(F("Task started - Red=A1, Green=A2"));
    log_info(F("Initial state - LOCKED (red solid)"));
}

void StatusLEDTask::on_msg(const MsgData& msg) {
    switch (msg.type) {
        case EVT_LED_LOCKED:
            if (currentState != LED_LOCKED) {
                currentState = LED_LOCKED;
                setLEDs(true, false); // Red solid, green off
                log_info(F("State changed to LOCKED (red solid)"));
            }
            break;
            
        case EVT_LED_UNLOCKED:
            if (currentState != LED_UNLOCKED) {
                currentState = LED_UNLOCKED;
                setLEDs(false, true); // Red off, green solid
                log_info(F("State changed to UNLOCKED (green solid)"));
            }
            break;
            
        case EVT_LED_TO_BE_LOCKED:
            if (currentState != LED_TO_BE_LOCKED) {
                currentState = LED_TO_BE_LOCKED;
                lastBlinkTime = OS.now();
                log_info(F("State changed to TO_BE_LOCKED (red blinking)"));
            }
            break;
            
        case EVT_LED_TO_BE_OPENED:
            if (currentState != LED_TO_BE_OPENED) {
                currentState = LED_TO_BE_OPENED;
                lastBlinkTime = OS.now();
                // Initialize green LED state for blinking
                greenLedState = true; // Start with green ON
                redLedState = false;  // Turn off red
                updateLEDs(); // Update immediately
                log_info(F("State changed to TO_BE_OPENED (green blinking)"));
            }
            break;
        
        case EVT_LED_CHILD_UNLOCKED:
            if (currentState != LED_CHILD_UNLOCKED) {
                currentState = LED_CHILD_UNLOCKED;
                lastBlinkTime = OS.now();
                // Indicate child lock disabled: alternate red/green blink
                altToggle = false;
                redLedState = true;
                greenLedState = false;
                updateLEDs();
                log_info(F("State changed to CHILD_UNLOCKED (alternating red/green)"));
            }
            break;
            
        default:
            // Ignore unknown message types
            break;
    }
}

void StatusLEDTask::step() {
    // Handle blinking for TO_BE_LOCKED and TO_BE_OPENED states
    if (currentState == LED_TO_BE_LOCKED || currentState == LED_TO_BE_OPENED || currentState == LED_CHILD_UNLOCKED) {
        unsigned long currentTime = OS.now();
        if (currentTime - lastBlinkTime >= BLINK_INTERVAL_MS) {
            if (currentState == LED_TO_BE_LOCKED) {
                // Toggle red LED for blinking
                redLedState = !redLedState;
                greenLedState = false; // Keep green off
            } else if (currentState == LED_TO_BE_OPENED) {
                // Toggle green LED for blinking
                greenLedState = !greenLedState;
                redLedState = false; // Keep red off
            } else if (currentState == LED_CHILD_UNLOCKED) {
                // Alternate red and green to indicate child lock disabled
                altToggle = !altToggle;
                redLedState = altToggle;
                greenLedState = !altToggle;
            }
            lastBlinkTime = currentTime;
            updateLEDs();
        }
    }
    
    // Process any received messages
}

void StatusLEDTask::setLEDs(bool red, bool green) {
    redLedState = red;
    greenLedState = green;
    updateLEDs();
}

void StatusLEDTask::updateLEDs() {
    digitalWrite(RED_LED_PIN, redLedState ? HIGH : LOW);
    digitalWrite(GREEN_LED_PIN, greenLedState ? HIGH : LOW);
}
