#include "SerialCommandTask.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <ctype.h>

// Case-insensitive compare: RAM string vs PROGMEM string
static bool equalsIgnoreCase_P(const char* ram, PGM_P rom) {
    while (true) {
        uint8_t c1 = (uint8_t)*ram++;
        uint8_t c2 = pgm_read_byte(rom++);
        uint8_t a = (uint8_t)tolower(c1);
        uint8_t b = (uint8_t)tolower(c2);
        if (a != b) return false;
        if (c2 == 0) return true;
    }
}

// Case-insensitive startsWith: does RAM string start with PROGMEM prefix?
static bool startsWithIgnoreCase_P(const char* ram, PGM_P rom) {
    while (true) {
        uint8_t c2 = pgm_read_byte(rom++);
        if (c2 == 0) return true; // matched full prefix
        uint8_t c1 = (uint8_t)*ram++;
        if ((uint8_t)tolower(c1) != (uint8_t)tolower(c2)) return false;
    }
}

SerialCommandTask::SerialCommandTask() {
    set_period(50); // Check for serial input every 50ms
    inputLen = 0;
}

void SerialCommandTask::on_start() {
    log_info(F("Task started"));
    Serial.println(F("Type 'help' for available commands"));
}

void SerialCommandTask::step() {
    // Read serial input
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (inputLen > 0) {
                inputBuffer[inputLen] = '\0';
                // Trim trailing spaces
                int end = (int)inputLen - 1;
                while (end >= 0 && (inputBuffer[end] == ' ' || inputBuffer[end] == '\t')) end--;
                inputBuffer[end + 1] = '\0';
                // Trim leading spaces by shifting
                int start = 0;
                while (inputBuffer[start] == ' ' || inputBuffer[start] == '\t') start++;
                if (start > 0) {
                    memmove(inputBuffer, inputBuffer + start, strlen(inputBuffer + start) + 1);
                }
                processCommand(inputBuffer);
                inputLen = 0;
            }
        } else if (inputLen < MAX_BUFFER_SIZE - 1) {
            inputBuffer[inputLen++] = c;
        }
    }
}

void SerialCommandTask::processCommand(const char* command) {
    Serial.print(F("> "));
    Serial.println(command);
    
    if (equalsIgnoreCase_P(command, PSTR("help")) || equalsIgnoreCase_P(command, PSTR("h"))) {
        printHelp();
    }
    else if (equalsIgnoreCase_P(command, PSTR("stats")) || equalsIgnoreCase_P(command, PSTR("s"))) {
        printTaskStats();
    }
    else if (equalsIgnoreCase_P(command, PSTR("reset")) || equalsIgnoreCase_P(command, PSTR("r"))) {
        printResetInfo();
    }
    else if (equalsIgnoreCase_P(command, PSTR("uptime")) || equalsIgnoreCase_P(command, PSTR("u"))) {
        printUptime();
    }
    else if (equalsIgnoreCase_P(command, PSTR("status")) || equalsIgnoreCase_P(command, PSTR("st"))) {
        printSystemStatus();
    }
    else if (startsWithIgnoreCase_P(command, PSTR("led "))) {
        handleLEDCommand(command + 4); // safe: advances over RAM buffer
    }
    else if (startsWithIgnoreCase_P(command, PSTR("light "))) {
        handleLightCommand(command + 6);
    }
    else if (startsWithIgnoreCase_P(command, PSTR("childlock "))) {
        handleChildLockCommand(command + 10);
    }
    else if (startsWithIgnoreCase_P(command, PSTR("password "))) {
        handlePasswordCommand(command + 9);
    }
    else if (startsWithIgnoreCase_P(command, PSTR("ledstate "))) {
        handleLEDStateCommand(command + 9);
    }
    else if (equalsIgnoreCase_P(command, PSTR("factoryreset"))) {
        handleFactoryResetCommand();
    }
    else if (equalsIgnoreCase_P(command, PSTR("sensors"))) {
        handleSensorStatus();
    }
    else if (equalsIgnoreCase_P(command, PSTR("memory")) || equalsIgnoreCase_P(command, PSTR("mem"))) {
        handleMemoryInfo();
    }
    else if (equalsIgnoreCase_P(command, PSTR("test")) || equalsIgnoreCase_P(command, PSTR("t"))) {
        handleKeypadTest();
    }
    else if (equalsIgnoreCase_P(command, PSTR("buzzer")) || equalsIgnoreCase_P(command, PSTR("b"))) {
        handleBuzzerTest();
    }
    else if (equalsIgnoreCase_P(command, PSTR("clear")) || equalsIgnoreCase_P(command, PSTR("c"))) {
        Serial.println(F("\033[2J\033[H")); // ANSI clear screen
    }
    else {
        printUnknownCommand(command);
    }
}

void SerialCommandTask::printHelp() {
    Serial.println(F("=== Available Commands ==="));
    Serial.println(F("help, h          - Show this help"));
    Serial.println(F("stats, s         - Show task statistics"));
    Serial.println(F("reset, r         - Show reset information"));
    Serial.println(F("uptime, u        - Show system uptime"));
    Serial.println(F("status, st       - Show system status"));
    Serial.println(F("led <state>      - Control LEDs (locked/unlocked/to_be_locked)"));
    Serial.println(F("ledstate <name>  - LED state (locked/unlocked/to_be_locked/to_be_opened/child_unlocked)"));
    Serial.println(F("light <cmd>      - Control light (on/off/toggle)"));
    Serial.println(F("childlock <cmd>  - Control child lock (engage/release/status/reset)"));
    Serial.println(F("password <cmd>   - Password ops (show/reload/set <4digits>/factory)"));
    Serial.println(F("factoryreset     - Reset EEPROM and defaults (DANGEROUS)"));
    Serial.println(F("sensors          - Show sensor status"));
    Serial.println(F("memory, mem      - Show memory usage information"));
    Serial.println(F("test, t          - Test keypad and button"));
    Serial.println(F("buzzer, b        - Test buzzer sounds"));
    Serial.println(F("clear, c         - Clear screen"));
    Serial.println(F(""));
    Serial.println(F("=== LED States ==="));
    Serial.println(F("led locked       - Red solid"));
    Serial.println(F("led unlocked     - Green solid"));
    Serial.println(F("led to_be_locked - Red blinking"));
    Serial.println(F(""));
    Serial.println(F("=== Light Commands ==="));
    Serial.println(F("light on         - Turn light on"));
    Serial.println(F("light off        - Turn light off"));
    Serial.println(F("light toggle     - Toggle light"));
    Serial.println(F(""));
    Serial.println(F("=== Child Lock Commands ==="));
    Serial.println(F("childlock engage - Engage child lock"));
    Serial.println(F("childlock release- Release child lock"));
    Serial.println(F("childlock status - Show child lock status"));
}

void SerialCommandTask::printTaskStats() {
    uint8_t taskCount = OS.get_task_count();
    Serial.print(F("=== Task Statistics ===\n"));
    Serial.print(F("Total tasks: "));
    Serial.println(taskCount);
    
    for (uint8_t i = 0; i < taskCount; i++) {
        TaskStats stats;
        if (OS.get_task_stats(i, stats)) {
            Task* task = OS.get_task(i);
            if (task) {
                Serial.print(F("Task "));
                Serial.print(i);
                Serial.print(F(" ("));
                Serial.print(task->get_name());
                Serial.print(F("): Runs="));
                Serial.print(stats.run_count);
                Serial.print(F(", MaxTime="));
                Serial.print(stats.max_exec_time_us);
                Serial.print(F("us, AvgTime="));
                if (stats.run_count > 0) {
                    Serial.print(stats.total_exec_time_us / stats.run_count);
                } else {
                    Serial.print(0);
                }
                Serial.print(F("us, Period="));
                Serial.print(task->get_period());
                Serial.println(F("ms"));
            }
        }
    }
}

void SerialCommandTask::printResetInfo() {
    ResetInfo resetInfo;
    if (OS.get_reset_info(resetInfo)) {
        Serial.println(F("=== Reset Information ==="));
        Serial.print(F("Reset Reason: "));
        Serial.print(resetInfo.reset_reason);
        Serial.print(F(", Last task: "));
        Serial.println(resetInfo.last_task_id);
    }
}

void SerialCommandTask::printUptime() {
    uint32_t uptime = OS.now();
    Serial.println(F("=== System Uptime ==="));
    Serial.print(F("Uptime: "));
    Serial.print(uptime / 1000);
    Serial.print(F("s ("));
    Serial.print(uptime);
    Serial.println(F("ms)"));
}

void SerialCommandTask::printSystemStatus() {
    Serial.println(F("=== System Status ==="));
    Serial.println(F("3D Printer Locker System"));
    Serial.println(F("Hardware:"));
    Serial.print(F("  Yellow Button: D"));
    Serial.println(YELLOW_BUTTON_PIN);
    Serial.print(F("  Keypad: D"));
    Serial.print(KEYPAD_PIN_1);
    Serial.print(F(", D"));
    Serial.print(KEYPAD_PIN_2);
    Serial.print(F(", D"));
    Serial.print(KEYPAD_PIN_3);
    Serial.print(F(", D"));
    Serial.println(KEYPAD_PIN_4);
    Serial.println(F("  Red LED: A1"));
    Serial.println(F("  Green LED: A2"));
    Serial.print(F("  Light Control: D"));
    Serial.println(LIGHT_PIN);
    Serial.print(F("  MB Light Sensor: D"));
    Serial.println(MB_LIGHT_SENSOR_PIN);
    Serial.println(F("  Device Running Sensor: A4"));
    Serial.print(F("  Child Lock Power: D"));
    Serial.println(CHILD_LOCK_POWER_PIN);
    Serial.println(F("  Child Lock Screen: A0"));
    Serial.println(F("Features: Watchdog, Reset tracking, Task monitoring, Smart child lock"));
}

void SerialCommandTask::handleLEDCommand(const char* args) {
    while (*args == ' ') args++;
    if (strcasecmp(args, "locked") == 0) {
        log_info(F("Setting LED to LOCKED (red solid)"));
        publish(EVT_LED_LOCKED, 0, 0, nullptr);
    }
    else if (strcasecmp(args, "unlocked") == 0) {
        log_info(F("Setting LED to UNLOCKED (green solid)"));
        publish(EVT_LED_UNLOCKED, 0, 0, nullptr);
    }
    else if (strcasecmp(args, "to_be_locked") == 0) {
        log_info(F("Setting LED to TO_BE_LOCKED (red blinking)"));
        publish(EVT_LED_TO_BE_LOCKED, 0, 0, nullptr);
    }
    else {
        Serial.println(F("Invalid LED state. Use: locked, unlocked, or to_be_locked"));
    }
}

void SerialCommandTask::handleKeypadTest() {
    Serial.println(F("=== Keypad Test ==="));
    Serial.println(F("Press keypad buttons 1-4 to test:"));
    Serial.println(F("  1: LOCKED (red solid)"));
    Serial.println(F("  2: UNLOCKED (green solid)"));
    Serial.println(F("  3: TO_BE_LOCKED (red blinking)"));
    Serial.println(F("  4: Cycle through states"));
    Serial.println(F("Press yellow button for short/long click test"));
    Serial.println(F("Type 'help' to return to command mode"));
}

void SerialCommandTask::handleBuzzerTest() {
    Serial.println(F("=== Buzzer Test ==="));
    Serial.println(F("Testing all buzzer sounds..."));
    
    // Test all buzzer sounds (no blocking delays - let buzzer task handle timing)
    Serial.println(F("1. Button press sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0, nullptr);
    
    Serial.println(F("2. Keypad press sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_KEYPAD_PRESS, 0, nullptr);
    
    Serial.println(F("3. Wrong password sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_WRONG_PASSWORD, 0, nullptr);
    
    Serial.println(F("4. Correct password sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_CORRECT_PASSWORD, 0, nullptr);
    
    Serial.println(F("5. Door released sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_RELEASED, 0, nullptr);
    
    Serial.println(F("6. Door closed sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_CLOSED, 0, nullptr);
    
    Serial.println(F("7. Angry sound..."));
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND, 0, nullptr);
    
    Serial.println(F("Buzzer test complete! (Sounds will play sequentially)"));
}

void SerialCommandTask::handleLightCommand(const char* args) {
    while (*args == ' ') args++;
    if (strcasecmp(args, "on") == 0) {
        log_info(F("Turning light ON"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 1, nullptr);
    }
    else if (strcasecmp(args, "off") == 0) {
        log_info(F("Turning light OFF"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 0, nullptr);
    }
    else if (strcasecmp(args, "toggle") == 0) {
        log_info(F("Toggling light"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 2, nullptr);
    }
    else {
        Serial.println(F("Invalid light command. Use: on, off, or toggle"));
    }
}

void SerialCommandTask::handleChildLockCommand(const char* args) {
    while (*args == ' ') args++;
    if (strcasecmp(args, "engage") == 0) {
        log_info(F("Engaging child lock"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0, nullptr);
    }
    else if (strcasecmp(args, "release") == 0) {
        log_info(F("Releasing child lock"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_RELEASE, 0, nullptr);
    }
    else if (strcasecmp(args, "reset") == 0) {
        log_info(F("Resetting child lock timeout"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_TIMEOUT_RESET, 0, nullptr);
    }
    else if (strcasecmp(args, "status") == 0) {
        Serial.println(F("Child lock status:"));
        Serial.println(F("  - Automatically managed based on device running state"));
        Serial.println(F("  - Use 'sensors' command to see current device state"));
    }
    else {
        Serial.println(F("Invalid child lock command. Use: engage, release, status, or reset"));
    }
}

void SerialCommandTask::handlePasswordCommand(const char* args) {
    while (*args == ' ') args++;
    if (strcasecmp(args, "show") == 0) {
        Serial.println(F("Password cannot be shown for security. Use 'password reload' or keypad change mode."));
    }
    else if (strcasecmp(args, "reload") == 0) {
        log_info(F("Requesting password reload from EEPROM"));
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_RELOAD_REQUEST, 0, nullptr);
    }
    else if (startsWithIgnoreCase_P(args, PSTR("set "))) {
        Serial.println(F("CLI set not supported; use keypad (Yellow after correct PIN)."));
    }
    else if (equalsIgnoreCase_P(args, PSTR("factory"))) {
        log_info(F("Forcing factory password now"));
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_SET_FACTORY, 0, nullptr);
    }
    else if (startsWithIgnoreCase_P(args, PSTR("inject "))) {
        const char* p = args + 7;
        // Publish keypad events for each digit 1-4
        while (*p) {
            uint8_t evt = 0;
            if (*p == '1') evt = EVT_KEYPAD_1_PRESSED;
            else if (*p == '2') evt = EVT_KEYPAD_2_PRESSED;
            else if (*p == '3') evt = EVT_KEYPAD_3_PRESSED;
            else if (*p == '4') evt = EVT_KEYPAD_4_PRESSED;
            else if (*p == ' ' || *p == '\t') { p++; continue; }
            else break;
            publish(TOPIC_KEYPAD_EVENTS, evt, 0, nullptr);
            p++;
        }
        Serial.println(F("Injected keypad digits to password manager topic."));
    }
    else {
        Serial.println(F("Invalid password command. Use: show, reload, or set <4digits>"));
    }
}

void SerialCommandTask::handleLEDStateCommand(const char* args) {
    while (*args == ' ') args++;
    if (strcasecmp(args, "locked") == 0) {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0, nullptr);
    }
    else if (strcasecmp(args, "unlocked") == 0) {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_UNLOCKED, 0, nullptr);
    }
    else if (strcasecmp(args, "to_be_locked") == 0) {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_LOCKED, 0, nullptr);
    }
    else if (strcasecmp(args, "to_be_opened") == 0) {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0, nullptr);
    }
    else if (strcasecmp(args, "child_unlocked") == 0) {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_CHILD_UNLOCKED, 0, nullptr);
    }
    else {
        Serial.println(F("Invalid ledstate. Use: locked/unlocked/to_be_locked/to_be_opened/child_unlocked"));
    }
}

void SerialCommandTask::handleFactoryResetCommand() {
    Serial.println(F("=== FACTORY RESET ==="));
    Serial.println(F("Resetting password to 1234, light OFF, brightness 100%, child lock engaged."));
    // Reset EEPROM: brightness at 100, light off, password default
    eeprom_write_byte((uint8_t*)0, 100); // brightness 100%
    eeprom_write_byte((uint8_t*)EEPROM_LIGHT_STATE_ADDR, 0); // light OFF
    // initialize password
    eeprom_write_byte((uint8_t*)EEPROM_PASSWORD_MAGIC_ADDR, EEPROM_PASSWORD_MAGIC_VAL);
    const char* def = DEFAULT_PASSWORD;
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
        eeprom_write_byte((uint8_t*)(EEPROM_PASSWORD_ADDR + i), (uint8_t)def[i]);
    }
    // Re-engage child lock and set LEDs
    publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0, nullptr);
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0, nullptr);
    // Notify password manager to reload
    publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_RELOAD_REQUEST, 0, nullptr);
    Serial.println(F("Factory reset complete."));
}

void SerialCommandTask::handleSensorStatus() {
    Serial.println(F("=== Sensor Status ==="));
    
    // Read current sensor states
    bool mbLightSensor = digitalRead(MB_LIGHT_SENSOR_PIN);
    bool deviceRunning = digitalRead(DEVICE_RUNNING_SENSOR_PIN);
    
    Serial.print(F("Motherboard Light Sensor (D"));
    Serial.print(MB_LIGHT_SENSOR_PIN);
    Serial.print(F("): "));
    Serial.println(mbLightSensor ? F("HIGH") : F("LOW"));
    
    Serial.print(F("Device Running Sensor (A4): "));
    Serial.println(deviceRunning ? F("RUNNING") : F("STOPPED"));
    
    Serial.println(F(""));
    Serial.println(F("Child Lock Logic:"));
    if (deviceRunning) {
        Serial.println(F("  Device RUNNING: Both screen and power button locked"));
    } else {
        Serial.println(F("  Device STOPPED: Power button enabled, screen locked"));
    }
}

void SerialCommandTask::handleMemoryInfo() {
    Serial.println(F("=== Memory Information ==="));
    
    // Get system memory info
    SystemMemoryInfo sys_info;
    if (OS.get_system_memory_info(sys_info)) {
        // RAM Usage
        Serial.println(F("\nRAM:"));
        Serial.print(F("  Total: "));
        Serial.print(sys_info.total_ram);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.free_ram);
        Serial.println(F(" bytes"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.total_ram - sys_info.free_ram);
        Serial.println(F(" bytes"));
        Serial.print(F("  Usage: "));
        Serial.print((sys_info.total_ram - sys_info.free_ram) * 100 / sys_info.total_ram);
        Serial.println(F("%"));
        
        // Heap Status
        Serial.println(F("\nHeap:"));
        Serial.print(F("  Size:        "));
        Serial.print(sys_info.heap_size);
        Serial.println(F(" bytes"));
        Serial.print(F("  Largest Free: "));
        Serial.print(sys_info.largest_block);
        Serial.println(F(" bytes"));
        Serial.print(F("  Fragments:   "));
        Serial.println(sys_info.heap_fragments);
        Serial.print(F("  Fragmentation: "));
        Serial.print(OS.get_heap_fragmentation());
        Serial.println(F("%"));
        
        // Stack Usage
        Serial.println(F("\nStack:"));
        Serial.print(F("  Size:  "));
        Serial.print(sys_info.stack_size);
        Serial.println(F(" bytes"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.stack_used);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.stack_free);
        Serial.println(F(" bytes"));
        
        // Task Memory
        Serial.println(F("\nTasks:"));
        Serial.print(F("  Count:  "));
        Serial.println(sys_info.total_tasks);
        Serial.print(F("  Memory: "));
        Serial.print(sys_info.task_memory);
        Serial.println(F(" bytes"));
        
        // Message System
        Serial.println(F("\nMessages:"));
        Serial.print(F("  Active: "));
        Serial.println(sys_info.active_messages);
        Serial.print(F("  Memory: "));
        Serial.print(sys_info.message_memory);
        Serial.println(F(" bytes"));
        
        // Flash Usage
        Serial.println(F("\nProgram Memory:"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.flash_used);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.flash_free);
        Serial.println(F(" bytes"));
        Serial.print(F("  Usage: "));
        Serial.print(sys_info.flash_used * 100 / (sys_info.flash_used + sys_info.flash_free));
        Serial.println(F("%"));
        
        // Task Details
        Serial.println(F("\nTask Details:"));
        Serial.println(F("============"));
        
        // Print info for each task
        for (uint8_t i = 0; i < OS.get_task_count(); i++) {
            Task* task = OS.get_task(i);
            if (task) {
                TaskMemoryInfo task_info;
                if (OS.get_task_memory_info(i, task_info)) {
                    Serial.print(F("\nTask '"));
                    Serial.print(task->get_name());
                    Serial.println(F("':"));
                    Serial.print(F("  Structure:    "));
                    Serial.print(task_info.task_struct_size);
                    Serial.println(F(" bytes"));
                    Serial.print(F("  Subscriptions: "));
                    Serial.print(task_info.subscription_size);
                    Serial.println(F(" bytes"));
                    Serial.print(F("  Queue:        "));
                    Serial.print(task_info.queue_size);
                    Serial.println(F(" bytes"));
                    Serial.print(F("  Total:        "));
                    Serial.print(task_info.total_allocated);
                    Serial.println(F(" bytes"));
                }
            }
        }
    } else {
        Serial.println(F("Failed to get system memory information"));
    }
    
    Serial.println(F(""));
}

void SerialCommandTask::printUnknownCommand(const char* command) {
    Serial.print(F("Unknown command: '"));
    Serial.print(command);
    Serial.println(F("'"));
    Serial.println(F("Type 'help' for available commands"));
}
