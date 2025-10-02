#include "BuzzerTask.h"

BuzzerTask::BuzzerTask() {
    set_period(50); // Check every 50ms for responsive sound control
    currentState = BUZZER_IDLE;
    soundStartTime = 0;
    soundDuration = 0;
    currentFrequency = 0;
    isPlaying = false;
}

void BuzzerTask::on_start() {
    // Initialize buzzer pin as output
    pinMode(BUZZER_PIN, OUTPUT);
    
    // Subscribe to buzzer events
    subscribe(TOPIC_BUZZER_EVENTS);
    
    // Start with buzzer off
    digitalWrite(BUZZER_PIN, LOW);
    
    log_info(F("Task started - Pin=A3"));
}

void BuzzerTask::on_msg(const MsgData& msg) {
    switch (msg.type) {
        case EVT_BUZZER_BUTTON_PRESS:
            playButtonPress();
            break;
            
        case EVT_BUZZER_KEYPAD_PRESS:
            playKeypadPress();
            break;
            
        case EVT_BUZZER_WRONG_PASSWORD:
            playWrongPassword();
            break;
            
        case EVT_BUZZER_CORRECT_PASSWORD:
            playCorrectPassword();
            break;
            
        case EVT_BUZZER_DOOR_RELEASED:
            playDoorReleased();
            break;
            
        case EVT_BUZZER_DOOR_CLOSED:
            playDoorClosed();
            break;
            
        case EVT_BUZZER_ANGRY_SOUND:
            playAngrySound();
            break;
            
        case EVT_BUZZER_ANGRY_SOUND_START:
            startAngrySound();
            break;
            
        case EVT_BUZZER_ANGRY_SOUND_STOP:
            stopAngrySound();
            break;
            
        case EVT_BUZZER_TOP_DOOR_SELECTED:
            playTopDoorSelected();
            break;
            
        case EVT_BUZZER_FRONT_DOOR_SELECTED:
            playFrontDoorSelected();
            break;
            
        case EVT_BUZZER_BOTH_DOORS_SELECTED:
            playBothDoorsSelected();
            break;
            
        case EVT_BUZZER_CHILD_LOCK_SELECTED:
            playChildLockSelected();
            break;
            
        default:
            break;
    }
}

void BuzzerTask::step() {
    unsigned long currentTime = OS.now();
    
    // Check if current sound should stop
    if (isPlaying && currentState == BUZZER_PLAYING) {
        if (currentTime - soundStartTime >= soundDuration) {
            stopSound();
        }
    }
    
    // Handle continuous angry sound
    if (currentState == BUZZER_ANGRY_CONTINUOUS) {
        if (currentTime - soundStartTime >= soundDuration) {
            // Restart the angry sound
            tone(BUZZER_PIN, currentFrequency, soundDuration);
            soundStartTime = currentTime;
        }
    }
    
    // Process any received messages
}

void BuzzerTask::playButtonPress() {
    startSound(800, 100); // Short beep
    log_debug(F("Button press sound"));
}

void BuzzerTask::playKeypadPress() {
    startSound(1000, 80); // Higher pitch, shorter beep
    log_debug(F("Keypad press sound"));
}

void BuzzerTask::playWrongPassword() {
    startSound(400, 500); // Low, long beep
    log_info(F("Wrong password sound"));
}

void BuzzerTask::playCorrectPassword() {
    startSound(1200, 200); // High, medium beep
    log_info(F("Correct password sound"));
}

void BuzzerTask::playDoorReleased() {
    startSound(1000, 300); // Medium beep
    log_info(F("Door released sound"));
}

void BuzzerTask::playDoorClosed() {
    startSound(600, 150); // Lower beep
    log_info(F("Door closed sound"));
}

void BuzzerTask::playAngrySound() {
    // Angry sound: low frequency, long duration
    startSound(300, 2000); // Very low, very long angry beep
    log_warn(F("Angry sound - unauthorized access"));
}

void BuzzerTask::startAngrySound() {
    // Start continuous angry sound
    currentState = BUZZER_ANGRY_CONTINUOUS;
    soundStartTime = OS.now();
    soundDuration = 1000; // 1 second per beep
    currentFrequency = 300; // Low frequency
    isPlaying = true;
    
    tone(BUZZER_PIN, currentFrequency, soundDuration);
    log_info(F("Continuous angry sound started - unauthorized access"));
}

void BuzzerTask::stopAngrySound() {
    // Stop continuous angry sound
    currentState = BUZZER_IDLE;
    isPlaying = false;
    noTone(BUZZER_PIN);
    log_info(F("Continuous angry sound stopped"));
}

void BuzzerTask::startSound(uint16_t frequency, unsigned long duration) {
    currentState = BUZZER_PLAYING;
    soundStartTime = OS.now();
    soundDuration = duration;
    currentFrequency = frequency;
    isPlaying = true;
    
    // Generate tone using Arduino tone() function
    tone(BUZZER_PIN, frequency, duration);
}

void BuzzerTask::stopSound() {
    currentState = BUZZER_IDLE;
    isPlaying = false;
    noTone(BUZZER_PIN); // Stop any playing tone
}

void BuzzerTask::playTopDoorSelected() {
    // Two short beeps for top door
    startSound(1000, 150);
    log_info(F("Top door selected sound"));
}

void BuzzerTask::playFrontDoorSelected() {
    // Three short beeps for front door
    startSound(1200, 100);
    log_info(F("Front door selected sound"));
}

void BuzzerTask::playBothDoorsSelected() {
    // Rising tone for both doors
    startSound(800, 200);
    log_info(F("Both doors selected sound"));
}

void BuzzerTask::playChildLockSelected() {
    // Four short beeps for child lock
    startSound(1500, 80);
    log_info(F("Child lock selected sound"));
}
