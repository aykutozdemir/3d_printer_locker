#include "PasswordManagerTask.h"

PasswordManagerTask::PasswordManagerTask() : Task(F("PasswordManager"))
{
    setPeriod(100); // Check every 100ms
    currentState = PASSWORD_IDLE;
    enteredPassword[0] = '\0'; // Initialize as empty string
    strcpy(correctPassword, DEFAULT_PASSWORD);
    digitCount = 0;
}

void PasswordManagerTask::on_start()
{
    // Subscribe to keypad events
    subscribe(TOPIC_KEYPAD_EVENTS);
    // Listen to yellow button short press for password change trigger
    subscribe(TOPIC_BUTTON_EVENTS);
    // Listen to door sensor events for session management
    subscribe(TOPIC_DOOR_SENSOR_EVENTS);

    logInfo(F("Task started - 4-digit password"));
    // Load password from EEPROM if initialized
    loadPasswordFromEEPROM();
    logInfof(F("Password: %s"), correctPassword);
}

void PasswordManagerTask::on_msg(const MsgData &msg)
{
    switch (msg.type)
    {
        case EVT_PASSWORD_RELOAD_REQUEST:
            loadPasswordFromEEPROM();
            break;
        case EVT_PASSWORD_SET_FACTORY:
            strcpy(correctPassword, DEFAULT_PASSWORD);
            savePasswordToEEPROM();
            logInfo(F("Password set to factory default"));
            break;
        case EVT_BUTTON_SHORT_CLICK:
            handleYellowShortPress();
            break;
        case EVT_KEYPAD_1_PRESSED:
        case EVT_KEYPAD_2_PRESSED:
        case EVT_KEYPAD_3_PRESSED:
        case EVT_KEYPAD_4_PRESSED:
            logInfo(F("Keypad event received"));
            if (currentState == PASSWORD_DOOR_SESSION_ACTIVE || currentState == PASSWORD_SCREEN_SESSION_ACTIVE)
            {
                // Handle direct door selection during active session
                uint8_t digit = msg.type - 9;

                // Special handling for +4 during screen session
                if (digit == 4 && currentState == PASSWORD_SCREEN_SESSION_ACTIVE)
                {
                    logInfo(F("Key4 -> extend screen timeout"));
                    screenTimeoutTimer = createTimerTyped<Timer16>(SCREEN_TIMEOUT_MS);
                    logInfo(F("Screen timeout extended by 1 minute"));
                }
                else
                {
                    handleDoorSelection(digit);
                }
            }
            else if (currentState == PASSWORD_IDLE || currentState == PASSWORD_ENTERING ||
                     currentState == PASSWORD_CHANGE_ENTER || currentState == PASSWORD_CHANGE_CONFIRM)
            {
                // Convert event type to digit (10->1, 11->2, 12->3, 13->4)
                uint8_t digit = msg.type - 9;
                enteredPassword[digitCount] = '0' + digit;
                enteredPassword[digitCount + 1] = '\0';
                digitCount++;
                digitTimeoutTimer = createTimerTyped<Timer16>(PASSWORD_DIGIT_TIMEOUT_MS);

                if (currentState == PASSWORD_IDLE)
                {
                    currentState = PASSWORD_ENTERING;
                    logInfo(F("Starting password entry"));
                }

                // Keypad press sound is now handled by KeypadTask

                logInfof(F("Digit %u (%u/4)"), digit, digitCount);

                if (digitCount >= PASSWORD_LENGTH)
                {
                    if (currentState == PASSWORD_ENTERING)
                    {
                        checkPassword();
                    }
                    else if (currentState == PASSWORD_CHANGE_ENTER)
                    {
                        // First entry complete. Copy to buffer and prompt for confirmation
                        strcpy(newPasswordBuffer, enteredPassword);
                        resetEntryBuffer();
                        currentState = PASSWORD_CHANGE_CONFIRM;
                        logInfo(F("Password change: enter again to confirm"));
                        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_PASSWORD_ACCEPT, 0);
                        // Give more time for password confirmation (5 seconds)
                        digitTimeoutTimer = createTimerTyped<Timer16>(5000);
                    }
                    else if (currentState == PASSWORD_CHANGE_CONFIRM)
                    {
                        // Compare with buffer
                        if (strcmp(enteredPassword, newPasswordBuffer) == 0)
                        {
                            // Save to EEPROM and update active password
                            strcpy(correctPassword, enteredPassword);
                            savePasswordToEEPROM();
                            logInfo(F("Password change successful"));
                            // Success beep (correct password sound)
                            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_PASSWORD_ACCEPT, 0);
                        }
                        else
                        {
                            logWarn(F("Password change mismatch"));
                            // Error beep (wrong password sound)
                            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND, 0);
                        }
                        // Exit change mode
                        resetPassword();
                    }
                }
            }
            else if (currentState == PASSWORD_WAITING_DOOR_SELECTION)
            {
                // Handle door selection after correct password
                uint8_t digit = msg.type - 9;
                handleDoorSelection(digit);
            }
            break;

        case EVT_KEYPAD_1_LONG_PRESSED:
            if (currentState == PASSWORD_SCREEN_SESSION_ACTIVE)
            {
                logInfo(F("Key1 long -> lock screen"));
                publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0);
                resetPassword();
            }
            break;

        case EVT_DOOR_FRONT_CLOSED:
        case EVT_DOOR_TOP_CLOSED:
            if (currentState == PASSWORD_DOOR_SESSION_ACTIVE)
            {
                logInfo(F("Door closed -> ending door session"));
                resetPassword();
            }
            break;

        default:
            break;
    }
}

void PasswordManagerTask::step()
{
    // Check for timeout
    if (currentState == PASSWORD_ENTERING || currentState == PASSWORD_CHANGE_ENTER || currentState == PASSWORD_CHANGE_CONFIRM)
    {
        if (digitTimeoutTimer.isExpired())
        {
            logWarn(F("Timeout - clearing password"));
            resetPassword();
        }
    }
    else if (currentState == PASSWORD_WAITING_DOOR_SELECTION)
    {
        if (selectionTimeoutTimer.isExpired())
        {
            logWarn(F("Selection timeout - no option selected"));
            resetPassword();
        }
    }
    else if (currentState == PASSWORD_SCREEN_SESSION_ACTIVE)
    {
        if (screenTimeoutTimer.isExpired())
        {
            logWarn(F("Screen timeout - locking screen"));
            publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0);
            resetPassword();
        }
    }
}

void PasswordManagerTask::resetPassword()
{
    enteredPassword[0] = '\0';
    digitCount = 0;
    currentState = PASSWORD_IDLE;
    logInfo(F("Reset to idle"));
}

void PasswordManagerTask::checkPassword()
{
    logInfof(F("Check %s vs %s"), enteredPassword, correctPassword);

    if (strcmp(enteredPassword, correctPassword) == 0)
    {
        currentState = PASSWORD_CORRECT;
        logInfo(F("CORRECT - waiting for selection"));
        logInfo(F("Press 1=Top, 2=Front, 3=Both, 4=Child Lock"));

        // Publish password correct event
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_CORRECT, 0);

        // Play correct password sound
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_PASSWORD_ACCEPT, 0);

        // Wait for door selection
        currentState = PASSWORD_WAITING_DOOR_SELECTION;
        selectionTimeoutTimer = createTimerTyped<Timer16>(PASSWORD_SELECTION_TIMEOUT_MS); // 5 seconds for selection
    }
    else
    {
        logWarn(F("WRONG - no action"));

        // Publish password wrong event
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_WRONG, 0);

        // Play wrong password sound
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND, 0);

        // Reset immediately (no blocking delay)
        resetPassword();
    }
}

void PasswordManagerTask::handleDoorSelection(uint8_t digit)
{
    logInfof(F("Door %u"), digit);

    switch (digit)
    {
        case 1:
            logInfo(F("Releasing TOP door"));
            publish(TOPIC_DOOR_EVENTS, EVT_DOOR_TOP_RELEASE, 0);
            break;

        case 2:
            logInfo(F("Releasing FRONT door"));
            publish(TOPIC_DOOR_EVENTS, EVT_DOOR_FRONT_RELEASE, 0);
            break;

        case 3:
            logInfo(F("Releasing BOTH doors"));
            publish(TOPIC_DOOR_EVENTS, EVT_DOOR_BOTH_RELEASE, 0);
            break;

        case 4:
            logInfo(F("Activating SCREEN/POWER BUTTON"));
            publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_RELEASE, 0);
            // Start screen session
            currentState = PASSWORD_SCREEN_SESSION_ACTIVE;
            screenTimeoutTimer = createTimerTyped<Timer16>(SCREEN_TIMEOUT_MS);
            logInfo(F("Screen session active - 1 minute timeout"));
            return; // Don't reset password for screen session
    }

    // For door selections (1,2,3), start door session
    if (digit >= 1 && digit <= 3)
    {
        currentState = PASSWORD_DOOR_SESSION_ACTIVE;
        logInfo(F("Door session active - until doors closed"));
    }
}

void PasswordManagerTask::handleYellowShortPress()
{
    // Allow entering change mode right after a correct password or during active session
    if (currentState == PASSWORD_WAITING_DOOR_SELECTION ||
            currentState == PASSWORD_DOOR_SESSION_ACTIVE ||
            currentState == PASSWORD_SCREEN_SESSION_ACTIVE)
    {
        logInfo(F("Password change mode initiated"));
        currentState = PASSWORD_CHANGE_ENTER;
        resetEntryBuffer();
        // Single event - buzzer will handle triple-beep pattern internally
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_PASSWORD_CHANGE, 0);
        // Give more time for password change (5 seconds instead of 3)
        digitTimeoutTimer = createTimerTyped<Timer16>(5000);
    }
}

void PasswordManagerTask::resetEntryBuffer()
{
    enteredPassword[0] = '\0';
    digitCount = 0;
    digitTimeoutTimer = createTimerTyped<Timer16>(PASSWORD_DIGIT_TIMEOUT_MS);
}

void PasswordManagerTask::loadPasswordFromEEPROM()
{
    uint8_t magic = eeprom_read_byte((uint8_t *)EEPROM_PASSWORD_MAGIC_ADDR);
    if (magic == EEPROM_PASSWORD_MAGIC_VAL)
    {
        char buf[PASSWORD_LENGTH + 1];
        for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
        {
            buf[i] = (char)eeprom_read_byte((uint8_t *)(EEPROM_PASSWORD_ADDR + i));
            if (buf[i] < '0' || buf[i] > '9')
            {
                buf[i] = '0';
            }
        }
        buf[PASSWORD_LENGTH] = '\0';
        strcpy(correctPassword, buf);
        logInfo(F("Loaded password from EEPROM"));
    }
    else
    {
        // Initialize EEPROM with default password
        savePasswordToEEPROM();
        logInfo(F("Initialized EEPROM with default"));
    }
}

void PasswordManagerTask::savePasswordToEEPROM()
{
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        eeprom_write_byte((uint8_t *)(EEPROM_PASSWORD_ADDR + i), (uint8_t)correctPassword[i]);
    }
    eeprom_write_byte((uint8_t *)EEPROM_PASSWORD_MAGIC_ADDR, EEPROM_PASSWORD_MAGIC_VAL);
}
