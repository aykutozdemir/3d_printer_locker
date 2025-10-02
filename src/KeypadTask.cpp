#include "KeypadTask.h"

KeypadTask::KeypadTask() : Task(nullptr) {
    set_period(10); // Check keypad every 10ms for responsive debouncing
}

void KeypadTask::on_start() {
    // Initialize keypad pins
    pinMode(KEYPAD_PIN_1, INPUT_PULLUP);
    pinMode(KEYPAD_PIN_2, INPUT_PULLUP);
    pinMode(KEYPAD_PIN_3, INPUT_PULLUP);
    pinMode(KEYPAD_PIN_4, INPUT_PULLUP);
    
    // Initialize key states
    for (uint8_t i = 0; i < 4; i++) {
        keys[i].lastState = HIGH;
        keys[i].currentState = HIGH;
        keys[i].lastPressTime = 0;
        keys[i].debounced = false;
        keys[i].longReported = false;
    }
    
    log_info(F("Task started - 4-digit keypad (1,2,3,4)"));
}

void KeypadTask::step() {
    // Check each key
    checkKey(0, KEYPAD_PIN_1, EVT_KEYPAD_1_PRESSED);
    checkKey(1, KEYPAD_PIN_2, EVT_KEYPAD_2_PRESSED);
    checkKey(2, KEYPAD_PIN_3, EVT_KEYPAD_3_PRESSED);
    checkKey(3, KEYPAD_PIN_4, EVT_KEYPAD_4_PRESSED);
}

void KeypadTask::checkKey(uint8_t keyIndex, uint8_t pin, uint8_t eventType) {
    keys[keyIndex].currentState = digitalRead(pin);
    
    // Detect key press (active low due to INPUT_PULLUP)
    if (keys[keyIndex].currentState == LOW && keys[keyIndex].lastState == HIGH) {
        // Key just pressed
        keys[keyIndex].lastPressTime = OS.now();
        keys[keyIndex].debounced = false;
        keys[keyIndex].longReported = false;
        log_debugf(F("Key %u press detected"), keyIndex + 1);
    }
    // Detect key release
    else if (keys[keyIndex].currentState == HIGH && keys[keyIndex].lastState == LOW) {
        // Key just released
        if (!keys[keyIndex].debounced) {
            unsigned long pressDuration = OS.now() - keys[keyIndex].lastPressTime;
            
            if (pressDuration >= DEBOUNCE_TIME_MS && !keys[keyIndex].longReported) {
                // Valid short press - send event
                handleKeyPress(keyIndex, eventType);
                keys[keyIndex].debounced = true;
            }
        }
        
        log_debugf(F("Key %u release detected"), keyIndex + 1);
    }
    // Handle long press while held
    else if (keys[keyIndex].currentState == LOW && !keys[keyIndex].longReported) {
        unsigned long pressDuration = OS.now() - keys[keyIndex].lastPressTime;
        if (pressDuration >= LONG_PRESS_TIME_MS) {
            // Report long press once
            uint8_t longEvent = 0;
            switch (eventType) {
                case EVT_KEYPAD_1_PRESSED: longEvent = EVT_KEYPAD_1_LONG_PRESSED; break;
                case EVT_KEYPAD_2_PRESSED: longEvent = EVT_KEYPAD_2_LONG_PRESSED; break;
                case EVT_KEYPAD_3_PRESSED: longEvent = EVT_KEYPAD_3_LONG_PRESSED; break;
                case EVT_KEYPAD_4_PRESSED: longEvent = EVT_KEYPAD_4_LONG_PRESSED; break;
            }
            if (longEvent != 0) {
                handleKeyLongPress(keyIndex, longEvent);
                keys[keyIndex].longReported = true;
                keys[keyIndex].debounced = true; // prevent short press on release
            }
        }
    }
    
    keys[keyIndex].lastState = keys[keyIndex].currentState;
}

void KeypadTask::handleKeyPress(uint8_t keyIndex, uint8_t eventType) {
    log_infof(F("Key %u pressed - sending event"), keyIndex + 1);
    
    // Publish key press event
    publish(TOPIC_KEYPAD_EVENTS, eventType, 0, nullptr);
    // Immediate beep for keypad press
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_KEYPAD_PRESS, 0, nullptr);
}

void KeypadTask::handleKeyLongPress(uint8_t keyIndex, uint8_t eventType) {
    log_infof(F("Key %u long pressed - sending event"), keyIndex + 1);
    publish(TOPIC_KEYPAD_EVENTS, eventType, 0, nullptr);
}
