#include "BuzzerTask.h"

BuzzerTask::BuzzerTask() : Task(F("Buzzer"))
{
    setPeriod(50); // Check every 50ms for responsive sound control
    currentState = BUZZER_IDLE;
    soundDuration = 0;
    currentFrequency = 0;
    isPlaying = false;
    beepCount = 0;
}

void BuzzerTask::on_start()
{
    // Initialize buzzer pin as output
    pinMode(BUZZER_PIN, OUTPUT);

    // Subscribe to buzzer events
    subscribe(TOPIC_BUZZER_EVENTS);

    // Start with buzzer off
    digitalWrite(BUZZER_PIN, LOW);

    logInfo(F("Started"));
}

void BuzzerTask::on_msg(const MsgData &msg)
{
    switch (msg.type)
    {
        case EVT_BUZZER_BUTTON_PRESS:
            playButtonPress();
            break;

        case EVT_BUZZER_PASSWORD_CHANGE:
            playPasswordChange();
            break;

        case EVT_BUZZER_PASSWORD_ACCEPT:
            playPasswordAccept();
            break;

        case EVT_BUZZER_DOOR_OPEN:
            playDoorOpen();
            break;

        case EVT_BUZZER_DOOR_CLOSE:
            playDoorClose();
            break;

        case EVT_BUZZER_ANGRY_SOUND:
            playAngrySound();
            break;

        default:
            break;
    }
}

void BuzzerTask::step()
{
    // Check if current sound should stop
    if (isPlaying && currentState == BUZZER_PLAYING)
    {
        if (soundTimer.isExpired())
        {
            stopSound();
        }
    }

    // Handle continuous angry sound
    if (currentState == BUZZER_ANGRY_CONTINUOUS)
    {
        if (soundTimer.isExpired())
        {
            soundTimer.startTimer(soundDuration);
            // Restart the angry sound
            tone(BUZZER_PIN, currentFrequency, soundDuration);
        }
    }

    // Handle double-beep pattern for password accept
    if (currentState == BUZZER_DOUBLE_BEEP)
    {
        if (beepTimer.isExpired())
        {
            if (beepCount < 2)
            {
                // Play a beep
                tone(BUZZER_PIN, 1200, 150); // High pitch, short beep
                beepCount++;

                if (beepCount < 2)
                {
                    // Wait 200ms between beeps
                    beepTimer = createTimerTyped<Timer16>(200);
                }
                else
                {
                    // All beeps done, return to idle
                    currentState = BUZZER_IDLE;
                }
            }
        }
    }

    // Handle triple-beep pattern for password change
    if (currentState == BUZZER_TRIPLE_BEEP)
    {
        if (beepTimer.isExpired())
        {
            if (beepCount < 3)
            {
                // Play a beep
                tone(BUZZER_PIN, 1200, 150); // High pitch, short beep
                beepCount++;

                if (beepCount < 3)
                {
                    // Wait 250ms between beeps
                    beepTimer = createTimerTyped<Timer16>(250);
                }
                else
                {
                    // All beeps done, return to idle
                    currentState = BUZZER_IDLE;
                }
            }
        }
    }

    // Process any received messages
}

void BuzzerTask::playButtonPress()
{
    startSound(1000, 150); // Longer, higher pitch beep for password change
    logDebug(F("Buzz btn"));
}

void BuzzerTask::playPasswordAccept()
{
    // Play distinctive double-beep for correct password
    currentState = BUZZER_DOUBLE_BEEP;
    beepCount = 0;
    beepTimer = createTimerTyped<Timer16>(200); // Start first beep immediately
    logInfo(F("Password accept - double beep"));
}

void BuzzerTask::playDoorOpen()
{
    // Play door open sound for all doors
    startSound(1000, 300); // Medium pitch, medium duration
    logInfo(F("Door open"));
}

void BuzzerTask::playDoorClose()
{
    // Play door close sound for all doors
    startSound(600, 150); // Lower pitch, shorter duration
    logInfo(F("Door close"));
}

void BuzzerTask::playPasswordChange()
{
    // Play a distinctive triple-beep pattern for password change mode
    // This will be handled by the step() function with proper timing
    currentState = BUZZER_TRIPLE_BEEP;
    beepCount = 0;
    beepTimer = createTimerTyped<Timer16>(200); // Start first beep immediately
    logInfo(F("Password change"));
}

void BuzzerTask::playAngrySound()
{
    startSound(300, 800); // Low pitch, shorter duration (was 2000ms)
    logWarn(F("Angry! unauthorized"));
}

void BuzzerTask::startSound(uint16_t frequency, unsigned long duration)
{
    currentState = BUZZER_PLAYING;
    soundTimer = createTimerTyped<Timer16>(duration);
    soundDuration = duration;
    currentFrequency = frequency;
    isPlaying = true;

    // Generate tone using Arduino tone() function
    tone(BUZZER_PIN, frequency, duration);
}

void BuzzerTask::stopSound()
{
    currentState = BUZZER_IDLE;
    isPlaying = false;
    noTone(BUZZER_PIN); // Stop any playing tone
}
