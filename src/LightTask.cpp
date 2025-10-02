#include "LightTask.h"

LightTask::LightTask() {
    set_period(50); // Update every 50ms for smooth dimming
    currentState = LIGHT_OFF;
    lightOn = false;
    currentDimLevel = 0;
    savedDimLevel = 50; // Default to 50% brightness
    lastDimStepTime = 0;
    dimIncreasing = true;
    manualOverride = false; // Start with MB sensor control
}

void LightTask::on_start() {
    // Initialize light pin as output
    pinMode(LIGHT_PIN, OUTPUT);
    
    // Configure Timer1 for 10-bit PWM on pin D10 (Channel B)
    // Set Timer1 to 10-bit Fast PWM mode (ICR1 = 1023)
    TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1); // Fast PWM, 10-bit, non-inverting
    TCCR1B = (1 << WGM12) | (1 << CS10); // Fast PWM, no prescaler (16MHz/1024 = ~15.6kHz)
    ICR1 = 1023; // Set TOP value for 10-bit resolution (0-1023)
    
    // Subscribe to light events
    subscribe(TOPIC_LIGHT_EVENTS);
    
    // Load saved dim level from EEPROM
    loadDimLevel();
    
    // Restore saved on/off state
    uint8_t state = eeprom_read_byte((uint8_t*)EEPROM_LIGHT_STATE_ADDR);
    if (state == 1) {
        currentDimLevel = savedDimLevel;
        setLightState(true);
        currentState = LIGHT_ON;
        log_info(F("Light restored ON from EEPROM"));
    } else {
        setLightState(false);
        currentState = LIGHT_OFF;
        log_info(F("Light restored OFF from EEPROM"));
    }
    
    log_info(F("Task started - Pin D10, 10-bit PWM (~15.6kHz)"));
    log_infof(F("Saved dim level: %u%%"), savedDimLevel);
}

void LightTask::on_msg(const MsgData& msg) {
    switch (msg.type) {
        case EVT_LIGHT_TOGGLE:
            if (msg.arg == 1) {
                // Force ON (from motherboard sensor)
                if (!manualOverride) { // Only if not in manual override mode
                    currentDimLevel = savedDimLevel;
                    setLightState(true);
                    currentState = LIGHT_ON;
                    log_infof(F("Forced ON at %u%% brightness (MB sensor)"), currentDimLevel);
                } else {
                    log_info(F("MB sensor ON ignored (manual override active)"));
                }
            } else if (msg.arg == 0) {
                // Force OFF (from motherboard sensor)
                if (!manualOverride) { // Only if not in manual override mode
                    setLightState(false);
                    currentState = LIGHT_OFF;
                    log_info(F("Forced OFF (MB sensor)"));
                } else {
                    log_info(F("MB sensor OFF ignored (manual override active)"));
                }
            } else {
                // Toggle (from button) - this sets manual override mode
                manualOverride = true; // Enable manual override
                if (currentState == LIGHT_OFF) {
                    // Turn on with saved dim level
                    currentDimLevel = savedDimLevel;
                    setLightState(true);
                    currentState = LIGHT_ON;
                    log_infof(F("Turned ON at %u%% brightness (manual override)"), currentDimLevel);
                } else {
                    // Turn off
                    setLightState(false);
                    currentState = LIGHT_OFF;
                    log_info(F("Turned OFF (manual override)"));
                }
            }
            break;
            
        case EVT_LIGHT_DIM_START:
            if (currentState == LIGHT_ON) {
                manualOverride = true; // Enable manual override for dimming
                currentState = LIGHT_DIMMING;
                lastDimStepTime = OS.now();
                dimIncreasing = true;
                log_info(F("Dimming mode started - cycling 0-100% every 1s (manual override)"));
            }
            break;
            
        case EVT_LIGHT_DIM_STOP:
            if (currentState == LIGHT_DIMMING) {
                // Save current dim level and return to ON state
                saveDimLevel();
                currentState = LIGHT_ON;
                log_infof(F("Dimming stopped - saved level: %u%%"), savedDimLevel);
            }
            break;
            
        default:
            // Ignore unknown message types
            break;
    }
}

void LightTask::step() {
    // Handle dimming cycle
    if (currentState == LIGHT_DIMMING) {
        unsigned long currentTime = OS.now();
        if (currentTime - lastDimStepTime >= DIM_STEP_INTERVAL_MS) {
            if (dimIncreasing) {
                currentDimLevel += DIM_STEP_PERCENT;
                if (currentDimLevel >= MAX_DIM_LEVEL) {
                    currentDimLevel = MAX_DIM_LEVEL;
                    dimIncreasing = false; // Start decreasing
                }
            } else {
                currentDimLevel -= DIM_STEP_PERCENT;
                if (currentDimLevel <= 0) {
                    currentDimLevel = 0;
                    dimIncreasing = true; // Start increasing
                }
            }
            
            setDimLevel(currentDimLevel);
            lastDimStepTime = currentTime;
            
            log_debugf(F("Dim level: %u%%"), currentDimLevel);
        }
    }
    
    // Process any received messages
}

void LightTask::setLightState(bool on) {
    lightOn = on;
    if (on) {
        setDimLevel(currentDimLevel);
    } else {
        OCR1B = 0; // Turn off completely using Timer1 Channel B
    }
    // Persist on/off state
    eeprom_write_byte((uint8_t*)EEPROM_LIGHT_STATE_ADDR, on ? 1 : 0);
}

void LightTask::setDimLevel(uint8_t level) {
    if (level > MAX_DIM_LEVEL) level = MAX_DIM_LEVEL;
    currentDimLevel = level;
    
    if (lightOn) {
        uint16_t pwmValue = dimLevelToPWM(level);
        // Use Timer1 Channel B register directly for 10-bit PWM
        OCR1B = pwmValue;
    }
}

void LightTask::saveDimLevel() {
    savedDimLevel = currentDimLevel;
    // Save to EEPROM (address 0 for light dim level)
    eeprom_write_byte((uint8_t*)0, savedDimLevel);
    log_infof(F("Dim level saved to EEPROM: %u%%"), savedDimLevel);
}

void LightTask::loadDimLevel() {
    // Load from EEPROM (address 0 for light dim level)
    savedDimLevel = eeprom_read_byte((uint8_t*)0);
    // Validate range
    if (savedDimLevel > MAX_DIM_LEVEL) {
        savedDimLevel = 50; // Default to 50% if invalid
    }
}

uint16_t LightTask::dimLevelToPWM(uint8_t level) {
    // Convert 0-100% to 0-1023 PWM value with linear brightness perception
    // Using a lookup table for smooth, linear perceived brightness
    
    if (level == 0) return 0;
    if (level >= 100) return 1023;
    
    // Lookup table for linear perceived brightness (0-100% to PWM values)
    // These values provide smooth, equal brightness progression
    static const uint16_t PROGMEM brightnessTable[101] = {
        0,    10,   20,   30,   40,   50,   60,   70,   80,   90,   // 0-9%
        100,  110,  120,  130,  140,  150,  160,  170,  180,  190,  // 10-19%
        200,  210,  220,  230,  240,  250,  260,  270,  280,  290,  // 20-29%
        300,  310,  320,  330,  340,  350,  360,  370,  380,  390,  // 30-39%
        400,  410,  420,  430,  440,  450,  460,  470,  480,  490,  // 40-49%
        500,  510,  520,  530,  540,  550,  560,  570,  580,  590,  // 50-59%
        600,  610,  620,  630,  640,  650,  660,  670,  680,  690,  // 60-69%
        700,  710,  720,  730,  740,  750,  760,  770,  780,  790,  // 70-79%
        800,  810,  820,  830,  840,  850,  860,  870,  880,  890,  // 80-89%
        900,  910,  920,  930,  940,  950,  960,  970,  980,  990,  // 90-99%
        1023  // 100%
    };
    
    return pgm_read_word(&brightnessTable[level]);
}
