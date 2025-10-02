#include "YellowButtonTask.h"

YellowButtonTask::YellowButtonTask() {
    set_period(10); // Check button every 10ms for responsive debouncing
}

void YellowButtonTask::on_start() {
    pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP);
    lastButtonState = HIGH;
    buttonPressed = false;
    pressStartTime = 0;
    longPressDetected = false;
    log_info(F("Task started"));
}

void YellowButtonTask::step() {
    bool currentButtonState = digitalRead(YELLOW_BUTTON_PIN);
    
    // Detect button press (active low due to INPUT_PULLUP)
    if (currentButtonState == LOW && lastButtonState == HIGH) {
        // Button just pressed - start debounce timer
        buttonPressed = true;
        pressStartTime = OS.now();
        longPressDetected = false;
        log_debug(F("Press detected"));
    }
    // Detect button release
    else if (currentButtonState == HIGH && lastButtonState == LOW) {
        // Button just released
        if (buttonPressed) {
            unsigned long pressDuration = OS.now() - pressStartTime;
            
            if (pressDuration >= DEBOUNCE_TIME_MS) { // Debounce check
                if (pressDuration < LONG_PRESS_TIME_MS && !longPressDetected) {
                    // Short click
                    log_info(F("Short click detected"));
                    publish(TOPIC_BUTTON_EVENTS, EVT_BUTTON_SHORT_CLICK, 0, nullptr);
                }
                // Long press already handled in the press-hold logic
            }
            
            buttonPressed = false;
            log_debug(F("Release detected"));
        }
    }
    // Handle long press while button is held
    else if (buttonPressed && currentButtonState == LOW) {
        unsigned long pressDuration = OS.now() - pressStartTime;
        
        if (pressDuration >= LONG_PRESS_TIME_MS && !longPressDetected) {
            longPressDetected = true;
            log_info(F("Long press detected"));
            publish(TOPIC_BUTTON_EVENTS, EVT_BUTTON_LONG_CLICK, 0, nullptr);
        }
    }
    
    lastButtonState = currentButtonState;
}
