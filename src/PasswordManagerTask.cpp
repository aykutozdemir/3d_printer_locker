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

    logInfo(F("Task started - 4-digit password system"));
    // Load password from EEPROM if initialized
    loadPasswordFromEEPROM();
    logInfof(F("Current password is %s"), correctPassword);
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
            logInfo(F("Password set to factory default in RAM and EEPROM"));
            break;
        case EVT_BUTTON_SHORT_CLICK:
            handleYellowShortPress();
            break;
        case EVT_KEYPAD_1_PRESSED:
        case EVT_KEYPAD_2_PRESSED:
        case EVT_KEYPAD_3_PRESSED:
        case EVT_KEYPAD_4_PRESSED:
            logInfo(F("Keypad event received!"));
            if (currentState == PASSWORD_IDLE || currentState == PASSWORD_ENTERING ||
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

                logInfof(F("Digit %u entered (%u/4)"), digit, digitCount);

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
                        logInfo(F("Password change: enter new password again to confirm"));
                        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
                    }
                    else if (currentState == PASSWORD_CHANGE_CONFIRM)
                    {
                        // Compare with buffer
                        if (strcmp(enteredPassword, newPasswordBuffer) == 0)
                        {
                            // Save to EEPROM and update active password
                            strcpy(correctPassword, enteredPassword);
                            savePasswordToEEPROM();
                            logInfo(F("Password change successful - saved to EEPROM"));
                            // Confirmation beep (reuse correct password sound)
                            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_CORRECT_PASSWORD, 0, nullptr);
                        }
                        else
                        {
                            logWarn(F("Password change mismatch - keeping old password"));
                            // Error beep (reuse wrong password sound)
                            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_WRONG_PASSWORD, 0, nullptr);
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
            logWarn(F("Timeout - clearing entered password"));
            resetPassword();
        }
    }
    else if (currentState == PASSWORD_WAITING_DOOR_SELECTION)
    {
        if (digitTimeoutTimer.isExpired())
        {
            logWarn(F("Timeout - no option selected, resetting"));
            resetPassword();
        }
    }

}

void PasswordManagerTask::resetPassword()
{
    enteredPassword[0] = '\0';
    digitCount = 0;
    currentState = PASSWORD_IDLE;
    logInfo(F("Reset to idle state"));
}

void PasswordManagerTask::checkPassword()
{
    logInfof(F("Checking %s against %s"), enteredPassword, correctPassword);

    if (strcmp(enteredPassword, correctPassword) == 0)
    {
        currentState = PASSWORD_CORRECT;
        logInfo(F("CORRECT - waiting for selection"));
        logInfo(F("Press 1=Top, 2=Front, 3=Both doors, 4=Child Lock"));

        // Publish password correct event
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_CORRECT, 0, nullptr);

        // Play correct password sound
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_CORRECT_PASSWORD, 0, nullptr);

        // Wait for door selection
        currentState = PASSWORD_WAITING_DOOR_SELECTION;
        digitTimeoutTimer = createTimerTyped<Timer16>(PASSWORD_DIGIT_TIMEOUT_MS); // Reset timeout for door selection
    }
    else
    {
        logWarn(F("WRONG - no action taken"));

        // Publish password wrong event
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_WRONG, 0, nullptr);

        // Play wrong password sound
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_WRONG_PASSWORD, 0, nullptr);

        // Reset immediately (no blocking delay)
        resetPassword();
    }
}

void PasswordManagerTask::handleDoorSelection(uint8_t digit)
{
    logInfof(F("Door selection %u"), digit);

    switch (digit)
    {
        case 1:
            logInfo(F("Releasing TOP door"));
            publish(TOPIC_DOOR_EVENTS, EVT_DOOR_TOP_RELEASE, 0, nullptr);
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_TOP_DOOR_SELECTED, 0, nullptr);
            break;

        case 2:
            logInfo(F("Releasing FRONT door"));
            publish(TOPIC_DOOR_EVENTS, EVT_DOOR_FRONT_RELEASE, 0, nullptr);
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_FRONT_DOOR_SELECTED, 0, nullptr);
            break;

        case 3:
            logInfo(F("Releasing BOTH doors"));
            publish(TOPIC_DOOR_EVENTS, EVT_DOOR_BOTH_RELEASE, 0, nullptr);
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BOTH_DOORS_SELECTED, 0, nullptr);
            break;

        case 4:
            logInfo(F("Releasing CHILD LOCK"));
            publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_RELEASE, 0, nullptr);
            publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_CHILD_LOCK_SELECTED, 0, nullptr);
            break;

        default:
            logWarn(F("Invalid door selection"));
            break;
    }

    // Reset after door selection
    resetPassword();
}

void PasswordManagerTask::handleYellowShortPress()
{
    // Only allow entering change mode right after a correct password
    if (currentState == PASSWORD_WAITING_DOOR_SELECTION)
    {
        logInfo(F("Password change mode initiated (yellow short press)"));
        currentState = PASSWORD_CHANGE_ENTER;
        resetEntryBuffer();
        // Triple short beep to indicate change mode
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
        publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
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
        logInfo(F("Initialized EEPROM with default password"));
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
