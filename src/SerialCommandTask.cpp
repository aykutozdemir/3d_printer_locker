#include "SerialCommandTask.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <ctype.h>

// Case-insensitive compare: RAM string vs PROGMEM string
static bool equalsIgnoreCase_P(const char *ram, PGM_P rom)
{
    while (true)
    {
        uint8_t c1 = (uint8_t) * ram++;
        uint8_t c2 = pgm_read_byte(rom++);
        uint8_t a = (uint8_t)tolower(c1);
        uint8_t b = (uint8_t)tolower(c2);
        if (a != b)
        {
            return false;
        }
        if (c2 == 0)
        {
            return true;
        }
    }
}

// Case-insensitive startsWith: does RAM string start with PROGMEM prefix?
static bool startsWithIgnoreCase_P(const char *ram, PGM_P rom)
{
    while (true)
    {
        uint8_t c2 = pgm_read_byte(rom++);
        if (c2 == 0)
        {
            return true;    // matched full prefix
        }
        uint8_t c1 = (uint8_t) * ram++;
        if ((uint8_t)tolower(c1) != (uint8_t)tolower(c2))
        {
            return false;
        }
    }
}

SerialCommandTask::SerialCommandTask() : Task(F("SerialCommand"))
{
    setPeriod(50); // Check for serial input every 50ms
    inputLen = 0;
}

void SerialCommandTask::on_start()
{
    logInfo(F("Started"));
    Serial.println(F("Type 'h' for help"));
}

void SerialCommandTask::step()
{
    // Read serial input
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\n' || c == '\r')
        {
            if (inputLen > 0)
            {
                inputBuffer[inputLen] = '\0';
                // Trim trailing spaces
                int end = (int)inputLen - 1;
                while (end >= 0 && (inputBuffer[end] == ' ' || inputBuffer[end] == '\t'))
                {
                    end--;
                }
                inputBuffer[end + 1] = '\0';
                // Trim leading spaces by shifting
                int start = 0;
                while (inputBuffer[start] == ' ' || inputBuffer[start] == '\t')
                {
                    start++;
                }
                if (start > 0)
                {
                    memmove(inputBuffer, inputBuffer + start, strlen(inputBuffer + start) + 1);
                }
                processCommand(inputBuffer);
                inputLen = 0;
            }
        }
        else if (inputLen < MAX_BUFFER_SIZE - 1)
        {
            inputBuffer[inputLen++] = c;
        }
    }
}

void SerialCommandTask::processCommand(const char *command)
{
    Serial.print(F("> "));
    Serial.println(command);

    if (equalsIgnoreCase_P(command, PSTR("help")) || equalsIgnoreCase_P(command, PSTR("h")))
    {
        printHelp();
    }
    else if (equalsIgnoreCase_P(command, PSTR("stats")) || equalsIgnoreCase_P(command, PSTR("s")))
    {
        printTaskStats();
    }
    else if (equalsIgnoreCase_P(command, PSTR("reset")) || equalsIgnoreCase_P(command, PSTR("r")))
    {
        printResetInfo();
    }
    else if (equalsIgnoreCase_P(command, PSTR("uptime")) || equalsIgnoreCase_P(command, PSTR("u")))
    {
        printUptime();
    }
    else if (equalsIgnoreCase_P(command, PSTR("status")) || equalsIgnoreCase_P(command, PSTR("st")))
    {
        printSystemStatus();
    }
    else if (startsWithIgnoreCase_P(command, PSTR("led ")))
    {
        handleLEDCommand(command + 4); // safe: advances over RAM buffer
    }
    else if (startsWithIgnoreCase_P(command, PSTR("light ")))
    {
        handleLightCommand(command + 6);
    }
    else if (startsWithIgnoreCase_P(command, PSTR("childlock ")))
    {
        handleChildLockCommand(command + 10);
    }
    else if (startsWithIgnoreCase_P(command, PSTR("password ")))
    {
        handlePasswordCommand(command + 9);
    }
    else if (startsWithIgnoreCase_P(command, PSTR("ledstate ")))
    {
        handleLEDStateCommand(command + 9);
    }
    else if (equalsIgnoreCase_P(command, PSTR("factoryreset")))
    {
        handleFactoryResetCommand();
    }
    else if (equalsIgnoreCase_P(command, PSTR("sensors")))
    {
        handleSensorStatus();
    }
    else if (equalsIgnoreCase_P(command, PSTR("memory")) || equalsIgnoreCase_P(command, PSTR("mem")))
    {
        handleMemoryInfo();
    }
    else if (equalsIgnoreCase_P(command, PSTR("test")) || equalsIgnoreCase_P(command, PSTR("t")))
    {
        handleKeypadTest();
    }
    else if (equalsIgnoreCase_P(command, PSTR("buzzer")) || equalsIgnoreCase_P(command, PSTR("b")))
    {
        handleBuzzerTest();
    }
    else if (equalsIgnoreCase_P(command, PSTR("clear")) || equalsIgnoreCase_P(command, PSTR("c")))
    {
        Serial.println(F("\033[2J\033[H")); // ANSI clear screen
    }
    else if (equalsIgnoreCase_P(command, PSTR("resetlight")) || equalsIgnoreCase_P(command, PSTR("rl")))
    {
        handleResetLightOverride();
    }
    else if (equalsIgnoreCase_P(command, PSTR("lightstatus")) || equalsIgnoreCase_P(command, PSTR("ls")))
    {
        handleLightStatus();
    }
    else if (equalsIgnoreCase_P(command, PSTR("tasklimit")) || equalsIgnoreCase_P(command, PSTR("tl")))
    {
        handleTaskLimitCheck();
    }
    else
    {
        printUnknownCommand(command);
    }
}

void SerialCommandTask::printHelp()
{
    Serial.println(F("=== Commands ==="));
    Serial.println(F("h-help s-stats r-reset u-uptime"));
    Serial.println(F("st-status led-ledstate light childlock"));
    Serial.println(F("password factoryreset sensors mem"));
    Serial.println(F("t-test b-buzzer c-clear rl-resetlight"));
    Serial.println(F("ls-lightstatus tl-tasklimit"));
    Serial.println(F(""));
    Serial.println(F("=== LED States ==="));
    Serial.println(F("led locked/unlocked/to_be_locked"));
    Serial.println(F(""));
    Serial.println(F("=== Light Commands ==="));
    Serial.println(F("light on/off/toggle"));
    Serial.println(F(""));
    Serial.println(F("=== Child Lock Commands ==="));
    Serial.println(F("childlock engage/release/status"));
}

void SerialCommandTask::printTaskStats()
{
    uint8_t taskCount = OS.getTaskCount();
    Serial.print(F("=== Task Stats ===\n"));
    Serial.print(F("Tasks: "));
    Serial.println(taskCount);

    for (uint8_t i = 0; i < taskCount; i++)
    {
        TaskStats stats;
        if (OS.getTaskStats(i, stats))
        {
            Task *task = OS.getTask(i);
            if (task)
            {
                Serial.print(F("Task "));
                Serial.print(i);
                Serial.print(F(" ("));
                Serial.print(Task::readTaskName(task));
                Serial.print(F("): Runs="));
                Serial.print(stats.runCount);
                Serial.print(F(", MaxTime="));
                Serial.print(stats.maxExecTimeUs);
                Serial.print(F("us, AvgTime="));
                if (stats.runCount > 0)
                {
                    Serial.print(stats.totalExecTimeUs / stats.runCount);
                }
                else
                {
                    Serial.print(0);
                }
                Serial.print(F("us, Period="));
                Serial.print(task->getPeriod());
                Serial.println(F("ms"));
            }
        }
    }
}

void SerialCommandTask::printResetInfo()
{
    ResetInfo resetInfo;
    if (OS.getResetInfo(resetInfo))
    {
        Serial.println(F("=== Reset Info ==="));
        Serial.print(F("Reset Reason: "));
        Serial.print(resetInfo.resetReason);
        Serial.print(F(", Last task: "));
        Serial.println(resetInfo.lastTaskId);
    }
}

void SerialCommandTask::printUptime()
{
    uint32_t uptime = OS.now();
    Serial.println(F("=== Uptime ==="));
    Serial.print(F("Uptime: "));
    Serial.print(uptime / 1000);
    Serial.print(F("s ("));
    Serial.print(uptime);
    Serial.println(F("ms)"));
}

void SerialCommandTask::printSystemStatus()
{
    Serial.println(F("=== Status ==="));
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

void SerialCommandTask::handleLEDCommand(const char *args)
{
    while (*args == ' ')
    {
        args++;
    }
    if (strcasecmp(args, "locked") == 0)
    {
        logInfo(F("LED LOCKED"));
        publish(EVT_LED_LOCKED, 0, 0);
    }
    else if (strcasecmp(args, "unlocked") == 0)
    {
        logInfo(F("LED UNLOCKED"));
        publish(EVT_LED_UNLOCKED, 0, 0);
    }
    else if (strcasecmp(args, "to_be_locked") == 0)
    {
        logInfo(F("LED TO_BE_LOCKED"));
        publish(EVT_LED_TO_BE_LOCKED, 0, 0);
    }
    else
    {
        Serial.println(F("Invalid LED state. Use: locked, unlocked, or to_be_locked"));
    }
}

void SerialCommandTask::handleKeypadTest()
{
    Serial.println(F("=== Keypad Test ==="));
    Serial.println(F("Press 1-4: LED states"));
    Serial.println(F("Yellow: short/long test"));
}

void SerialCommandTask::handleBuzzerTest()
{
    Serial.println(F("=== Buzzer Test ==="));
    Serial.println(F("Testing sounds..."));

    // Test simplified buzzer sounds
    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_BUTTON_PRESS, 0);
    delay(1000);

    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_PASSWORD_CHANGE, 0);
    delay(2000); // Wait for triple-beep to complete

    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_PASSWORD_ACCEPT, 0);
    delay(1000);

    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_OPEN, 0);
    delay(1000);

    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_DOOR_CLOSE, 0);
    delay(1000);

    publish(TOPIC_BUZZER_EVENTS, EVT_BUZZER_ANGRY_SOUND, 0);
    delay(1000);

    Serial.println(F("Test complete!"));
}

void SerialCommandTask::handleResetLightOverride()
{
    Serial.println(F("=== Reset Light ==="));
    Serial.println(F("Resetting manual override"));

    // Send a special message to reset manual override
    // We'll use a new event type for this
    publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_RESET_OVERRIDE, 0);

    Serial.println(F("Override reset complete"));
}

void SerialCommandTask::handleLightStatus()
{
    Serial.println(F("=== Light Status ==="));
    Serial.println(F("Check for 'MB sensor ON ignored' message"));
}

void SerialCommandTask::handleLightCommand(const char *args)
{
    while (*args == ' ')
    {
        args++;
    }
    if (strcasecmp(args, "on") == 0)
    {
        logInfof(F("Light %S"), F("ON"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 1);
    }
    else if (strcasecmp(args, "off") == 0)
    {
        logInfof(F("Light %S"), F("OFF"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 0);
    }
    else if (strcasecmp(args, "toggle") == 0)
    {
        logInfo(F("Toggling light"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 2);
    }
    else
    {
        Serial.println(F("Invalid light command. Use: on, off, or toggle"));
    }
}

void SerialCommandTask::handleChildLockCommand(const char *args)
{
    while (*args == ' ')
    {
        args++;
    }
    if (strcasecmp(args, "engage") == 0)
    {
        logInfo(F("Engaging child lock"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0);
    }
    else if (strcasecmp(args, "release") == 0)
    {
        logInfo(F("Releasing child lock"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_RELEASE, 0);
    }
    else if (strcasecmp(args, "reset") == 0)
    {
        logInfo(F("Resetting child lock timeout"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_TIMEOUT_RESET, 0);
    }
    else if (strcasecmp(args, "status") == 0)
    {
        Serial.println(F("Child lock status:"));
        Serial.println(F("  - Automatically managed based on device running state"));
        Serial.println(F("  - Use 'sensors' command to see current device state"));
    }
    else
    {
        Serial.println(F("Invalid child lock command. Use: engage, release, status, or reset"));
    }
}

void SerialCommandTask::handlePasswordCommand(const char *args)
{
    while (*args == ' ')
    {
        args++;
    }
    if (strcasecmp(args, "show") == 0)
    {
        Serial.println(F("Password cannot be shown for security. Use 'password reload' or keypad change mode."));
    }
    else if (strcasecmp(args, "reload") == 0)
    {
        logInfo(F("Requesting password reload from EEPROM"));
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_RELOAD_REQUEST, 0);
    }
    else if (startsWithIgnoreCase_P(args, PSTR("set ")))
    {
        Serial.println(F("CLI set not supported; use keypad (Yellow after correct PIN)."));
    }
    else if (equalsIgnoreCase_P(args, PSTR("factory")))
    {
        logInfo(F("Forcing factory password now"));
        publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_SET_FACTORY, 0);
    }
    else if (startsWithIgnoreCase_P(args, PSTR("inject ")))
    {
        const char *p = args + 7;
        // Publish keypad events for each digit 1-4
        while (*p)
        {
            uint8_t evt = 0;
            if (*p == '1')
            {
                evt = EVT_KEYPAD_1_PRESSED;
            }
            else if (*p == '2')
            {
                evt = EVT_KEYPAD_2_PRESSED;
            }
            else if (*p == '3')
            {
                evt = EVT_KEYPAD_3_PRESSED;
            }
            else if (*p == '4')
            {
                evt = EVT_KEYPAD_4_PRESSED;
            }
            else if (*p == ' ' || *p == '\t')
            {
                p++;
                continue;
            }
            else
            {
                break;
            }
            publish(TOPIC_KEYPAD_EVENTS, evt, 0);
            p++;
        }
        Serial.println(F("Injected keypad digits to password manager topic."));
    }
    else
    {
        Serial.println(F("Invalid password command. Use: show, reload, or set <4digits>"));
    }
}

void SerialCommandTask::handleLEDStateCommand(const char *args)
{
    while (*args == ' ')
    {
        args++;
    }
    if (strcasecmp(args, "locked") == 0)
    {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0);
    }
    else if (strcasecmp(args, "unlocked") == 0)
    {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_UNLOCKED, 0);
    }
    else if (strcasecmp(args, "to_be_locked") == 0)
    {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_LOCKED, 0);
    }
    else if (strcasecmp(args, "to_be_opened") == 0)
    {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_TO_BE_OPENED, 0);
    }
    else if (strcasecmp(args, "child_unlocked") == 0)
    {
        publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_CHILD_UNLOCKED, 0);
    }
    else
    {
        Serial.println(F("Invalid ledstate. Use: locked/unlocked/to_be_locked/to_be_opened/child_unlocked"));
    }
}

void SerialCommandTask::handleFactoryResetCommand()
{
    Serial.println(F("=== FACTORY RESET ==="));
    Serial.println(F("Resetting password to 1234, light OFF, brightness 100%, child lock engaged."));
    // Reset EEPROM: brightness at 100, light off, password default
    eeprom_write_byte((uint8_t *)0, 100); // brightness 100%
    eeprom_write_byte((uint8_t *)EEPROM_LIGHT_STATE_ADDR, 0); // light OFF
    // initialize password
    eeprom_write_byte((uint8_t *)EEPROM_PASSWORD_MAGIC_ADDR, EEPROM_PASSWORD_MAGIC_VAL);
    const char *def = DEFAULT_PASSWORD;
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        eeprom_write_byte((uint8_t *)(EEPROM_PASSWORD_ADDR + i), (uint8_t)def[i]);
    }
    // Re-engage child lock and set LEDs
    publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0);
    publish(TOPIC_STATUS_LED_EVENTS, EVT_LED_LOCKED, 0);
    // Notify password manager to reload
    publish(TOPIC_PASSWORD_EVENTS, EVT_PASSWORD_RELOAD_REQUEST, 0);
    Serial.println(F("Factory reset complete."));
}

void SerialCommandTask::handleSensorStatus()
{
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
    if (deviceRunning)
    {
        Serial.println(F("  Device RUNNING: Both screen and power button locked"));
    }
    else
    {
        Serial.println(F("  Device STOPPED: Power button enabled, screen locked"));
    }
}

void SerialCommandTask::handleMemoryInfo()
{
    Serial.println(F("=== Memory ==="));

    // Get system memory info
    SystemMemoryInfo sys_info;
    if (OS.getSystemMemoryInfo(sys_info))
    {
        // RAM Usage
        Serial.println(F("\nRAM:"));
        Serial.print(F("  Total: "));
        Serial.print(sys_info.totalRam);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.freeRam);
        Serial.println(F(" bytes"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.totalRam - sys_info.freeRam);
        Serial.println(F(" bytes"));
        Serial.print(F("  Usage: "));
        Serial.print((uint32_t)(sys_info.totalRam - sys_info.freeRam) * 100UL / (uint32_t)sys_info.totalRam);
        Serial.println(F("%"));

        // Heap Status
        Serial.println(F("\nHeap:"));
        Serial.print(F("  Size: "));
        Serial.print(sys_info.heapSize);
        Serial.println(F(" bytes"));

        // Stack Usage
        Serial.println(F("\nStack:"));
        Serial.print(F("  Size:  "));
        Serial.print(sys_info.stackSize);
        Serial.println(F(" bytes"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.stackUsed);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.stackFree);
        Serial.println(F(" bytes"));

        // Task Memory
        Serial.println(F("\nTasks:"));
        Serial.print(F("  Count:  "));
        Serial.println(sys_info.totalTasks);

        // Message System
        Serial.println(F("\nMessages:"));
        Serial.print(F("  Active: "));
        Serial.println(sys_info.activeMessages);

        // Flash Usage
        Serial.println(F("\nProgram Memory:"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.flashUsed);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.flashFree);
        Serial.println(F(" bytes"));
        Serial.print(F("  Usage: "));
        if (sys_info.flashUsed + sys_info.flashFree > 0)
        {
            Serial.print((uint32_t)sys_info.flashUsed * 100UL / (uint32_t)(sys_info.flashUsed + sys_info.flashFree));
        }
        else
        {
            Serial.print(F("N/A"));
        }
        Serial.println(F("%"));

        // EEPROM Usage
        Serial.println(F("\nEEPROM:"));
        Serial.print(F("  Used:  "));
        Serial.print(sys_info.eepromUsed);
        Serial.println(F(" bytes"));
        Serial.print(F("  Free:  "));
        Serial.print(sys_info.eepromFree);
        Serial.println(F(" bytes"));

        // Memory Leak Detection Stats
        MemoryStats leak_stats;
        if (OS.getMemoryLeakStats(leak_stats))
        {
            Serial.println(F("\nMemory Leak Detection:"));
            Serial.print(F("  Total Allocated: "));
            Serial.print(leak_stats.total_allocated);
            Serial.println(F(" bytes"));
            Serial.print(F("  Total Freed:      "));
            Serial.print(leak_stats.total_freed);
            Serial.println(F(" bytes"));
            Serial.print(F("  Current Usage:    "));
            Serial.print(leak_stats.current_usage);
            Serial.println(F(" bytes"));
            Serial.print(F("  Peak Usage:       "));
            Serial.print(leak_stats.peak_usage);
            Serial.println(F(" bytes"));

            // Calculate leak detection
            if (leak_stats.total_allocated > leak_stats.total_freed)
            {
                Serial.print(F("  Potential Leak:   "));
                Serial.print(leak_stats.total_allocated - leak_stats.total_freed);
                Serial.println(F(" bytes"));
            }
            else
            {
                Serial.println(F("  No leaks detected"));
            }
        }
        Serial.print(F("  Usage: "));
        if (sys_info.eepromUsed + sys_info.eepromFree > 0)
        {
            Serial.print((uint32_t)sys_info.eepromUsed * 100UL / (uint32_t)(sys_info.eepromUsed + sys_info.eepromFree));
        }
        else
        {
            Serial.print(F("N/A"));
        }
        Serial.println(F("%"));

        // Task Details
        Serial.println(F("\nTask Details:"));
        Serial.println(F("============"));

        // Print info for each task
        for (uint8_t i = 0; i < OS.getTaskCount(); i++)
        {
            Task *task = OS.getTask(i);
            if (task)
            {
                TaskMemoryInfo task_info;
                if (OS.getTaskMemoryInfo(i, task_info))
                {
                    Serial.print(F("\nTask '"));
                    Serial.print(Task::readTaskName(task));
                    Serial.println(F("':"));
                    Serial.print(F("  Structure:    "));
                    Serial.print(task_info.task_struct_size);
                    Serial.println(F(" bytes"));
                    Serial.print(F("  Subscriptions: "));
                    Serial.print(task_info.subscription_size);
                    Serial.println(F(" bytes"));
                    Serial.print(F("  Total:        "));
                    Serial.print(task_info.total_allocated);
                    Serial.println(F(" bytes"));
                }
            }
        }
    }
    else
    {
        Serial.println(F("Failed to get system memory information"));
    }

    Serial.println(F(""));
}

void SerialCommandTask::printUnknownCommand(const char *command)
{
    Serial.print(F("Unknown command: '"));
    Serial.print(command);
    Serial.println(F("'"));
    Serial.println(F("Type 'help' for available commands"));
}

void SerialCommandTask::handleTaskLimitCheck()
{
    uint8_t currentTasks = OS.getTaskCount();

    Serial.println(F("\n=== Task Limit Check ==="));
    Serial.print(F("Current tasks: "));
    Serial.println(currentTasks);
    Serial.print(F("Maximum tasks: "));
    Serial.println(MAX_TOPICS);
    Serial.print(F("Remaining slots: "));
    Serial.println(MAX_TOPICS - currentTasks);

    if (currentTasks >= MAX_TOPICS)
    {
        Serial.println(F("⚠️  TASK LIMIT REACHED!"));
        Serial.println(F("Cannot add more tasks."));
    }
    else if (currentTasks >= MAX_TOPICS - 2)
    {
        Serial.println(F("⚠️  WARNING: Near task limit!"));
        Serial.print(F("Only "));
        Serial.print(MAX_TOPICS - currentTasks);
        Serial.println(F(" slots remaining."));
    }
    else
    {
        Serial.println(F("✅ Task limit OK"));
    }

    Serial.println(F(""));
}
