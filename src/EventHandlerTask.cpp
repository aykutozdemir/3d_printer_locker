#include "EventHandlerTask.h"

EventHandlerTask::EventHandlerTask() : Task(nullptr) {
    set_period(100); // Check for messages every 100ms
    isDimming = false;
}

void EventHandlerTask::on_start() {
    // Subscribe to button events and motherboard light sensor events
    subscribe(TOPIC_BUTTON_EVENTS);
    subscribe(TOPIC_MB_LIGHT_SENSOR_EVENTS);
    log_info(F("Task started, listening for button and MB light sensor events"));
}

void EventHandlerTask::on_msg(const MsgData& msg) {
    switch (msg.type) {
        case EVT_BUTTON_SHORT_CLICK:
            if (isDimming) {
                log_info(F("EVENT: Short click received - stopping dimming and saving level"));
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_DIM_STOP, 0, nullptr);
                isDimming = false;
            } else {
                log_info(F("EVENT: Short click received - toggling light"));
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 2, nullptr); // arg=2 means toggle
            }
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
            break;
            
        case EVT_BUTTON_LONG_CLICK:
            log_info(F("EVENT: Long click received - starting dimming mode"));
            publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_DIM_START, 0, nullptr);
            isDimming = true;
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
            break;
            
        case EVT_MB_LIGHT_SENSOR_CHANGED:
            // Motherboard light sensor changed - control light based on sensor state
            if (msg.arg == 1) {
                log_info(F("EVENT: MB light sensor HIGH - turning light ON"));
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 1, nullptr); // Force ON
            } else {
                log_info(F("EVENT: MB light sensor LOW - turning light OFF"));
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 0, nullptr); // Force OFF
            }
            break;
            
        // Keypad events are now handled by PasswordManagerTask
            
        default:
            // Ignore unknown message types
            break;
    }
}

void EventHandlerTask::step() {
    // Process any received messages
}
