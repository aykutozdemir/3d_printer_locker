#include "EventHandlerTask.h"

EventHandlerTask::EventHandlerTask() : Task(F("EventHandler"))
{
    setPeriod(100); // Check for messages every 100ms
    isDimming = false;
}

void EventHandlerTask::on_start()
{
    // Subscribe to button events and motherboard light sensor events
    subscribe(TOPIC_BUTTON_EVENTS);
    subscribe(TOPIC_MB_LIGHT_SENSOR_EVENTS);
    logInfo(F("Started, listening for button and MB events"));
}

void EventHandlerTask::on_msg(const MsgData &msg)
{
    switch (msg.type)
    {
        case EVT_BUTTON_SHORT_CLICK:
            if (isDimming)
            {
                logInfo(F("EVENT: Short click - stopping dimming"));
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_DIM_STOP, 0);
                isDimming = false;
            }
            else
            {
                logInfo(F("EVENT: Short click - toggling light"));
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 2); // arg=2 means toggle
            }
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0);
            break;

        case EVT_BUTTON_LONG_CLICK:
            logInfo(F("EVENT: Long click - starting dimming"));
            publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_DIM_START, 0);
            isDimming = true;
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0);
            break;

        case EVT_MB_LIGHT_SENSOR_CHANGED:
            // Motherboard light sensor changed - control light based on sensor state
            logInfof(F("EVENT: MB sensor %S - light %S"),
                     msg.arg == 1 ? F("HIGH") : F("LOW"),
                     msg.arg == 1 ? F("ON") : F("OFF"));
            if (msg.arg == 1)
            {
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 1); // Force ON
            }
            else
            {
                publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 0); // Force OFF
            }
            break;

        // Keypad events are now handled by PasswordManagerTask

        default:
            // Ignore unknown message types
            break;
    }
}

void EventHandlerTask::step()
{
    // Process any received messages
}
