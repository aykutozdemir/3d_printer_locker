#include "YellowButtonTask.h"

YellowButtonTask::YellowButtonTask() : Task(F("YellowButton"))
{
    setPeriod(10); // Check button every 10ms for responsive debouncing
}

void YellowButtonTask::on_start()
{
    pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP);
    lastButtonState = HIGH;
    buttonPressed = false;
    // Timers will be initialized when button is pressed
    longPressDetected = false;
    logInfo(F("Started"));
}

void YellowButtonTask::step()
{
    bool currentButtonState = digitalRead(YELLOW_BUTTON_PIN);

    // Detect button press (active low due to INPUT_PULLUP)
    if (currentButtonState == LOW && lastButtonState == HIGH)
    {
        // Button just pressed - start debounce timer
        buttonPressed = true;
        debounceTimer = createTimerTyped<Timer8>(DEBOUNCE_TIME_MS);
        longPressTimer = createTimerTyped<Timer16>(LONG_PRESS_TIME_MS);
        longPressDetected = false;
        logDebug(F("Press"));
    }
    // Detect button release
    else if (currentButtonState == HIGH && lastButtonState == LOW)
    {
        // Button just released
        if (buttonPressed)
        {
            if (debounceTimer.isExpired())   // Debounce check
            {
                if (!longPressDetected)
                {
                    // Short click
                    logInfo(F("Short click"));
                    publish(TOPIC_BUTTON_EVENTS, EVT_BUTTON_SHORT_CLICK, 0);
                }
                // Long press already handled in the press-hold logic
            }

            buttonPressed = false;
            logDebug(F("Release"));
        }
    }
    // Handle long press while button is held
    else if (buttonPressed && currentButtonState == LOW)
    {
        if (longPressTimer.isExpired() && !longPressDetected)
        {
            longPressDetected = true;
            logInfo(F("Long press"));
            publish(TOPIC_BUTTON_EVENTS, EVT_BUTTON_LONG_CLICK, 0);
        }
    }

    lastButtonState = currentButtonState;
}
