#include "SerialCommandTask.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <ctype.h>

// Memory utility functions
int freeMemory()
{
    // Simple stack pointer method
    extern char __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

uint16_t getProgramSize()
{
    // This is a rough estimate - actual program size varies
    return 29800; // Current approximate size from compilation
}

uint16_t getActualEEPROMUsage()
{
    uint16_t used = 0;

    // Check if light brightness is set (address 0)
    uint8_t brightness = eeprom_read_byte((uint8_t *)0);
    if (brightness != 0xFF)
    {
        used++;    // 0xFF means uninitialized
    }

    // Check if light state is set (address 1)
    uint8_t lightState = eeprom_read_byte((uint8_t *)EEPROM_LIGHT_STATE_ADDR);
    if (lightState != 0xFF)
    {
        used++;
    }

    // Check if password magic is set (address 16)
    uint8_t magic = eeprom_read_byte((uint8_t *)EEPROM_PASSWORD_MAGIC_ADDR);
    if (magic == EEPROM_PASSWORD_MAGIC_VAL)
    {
        used++;
    }

    // Check if password is set (address 17-20)
    bool passwordSet = false;
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        uint8_t pwdByte = eeprom_read_byte((uint8_t *)(EEPROM_PASSWORD_ADDR + i));
        if (pwdByte != 0xFF && pwdByte >= '0' && pwdByte <= '9')
        {
            passwordSet = true;
            break;
        }
    }
    if (passwordSet)
    {
        used += PASSWORD_LENGTH;
    }

    return used;
}

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
    else if (equalsIgnoreCase_P(command, PSTR("resetlight")) || equalsIgnoreCase_P(command, PSTR("rl")))
    {
        handleResetLightOverride();
    }
    else if (equalsIgnoreCase_P(command, PSTR("lightstatus")) || equalsIgnoreCase_P(command, PSTR("ls")))
    {
        handleLightStatus();
    }
    else
    {
        printUnknownCommand(command);
    }
}

void SerialCommandTask::printHelp()
{
    Serial.println(F("=== SERIAL COMMANDS ==="));
    Serial.println(F(""));
    Serial.println(F("System Information:"));
    Serial.println(F("  h, help     - Show this help"));
    Serial.println(F("  s, stats    - Show task statistics"));
    Serial.println(F("  r, reset    - Show reset information"));
    Serial.println(F("  u, uptime   - Show system uptime"));
    Serial.println(F("  st, status  - Show system status"));
    Serial.println(F("  mem, memory - Show memory usage and task RAM usage"));
    Serial.println(F(""));
    Serial.println(F("Hardware Control:"));
    Serial.println(F("  led <state> - Control status LED"));
    Serial.println(F("    locked, unlocked, to_be_locked"));
    Serial.println(F("  light <cmd> - Control internal light"));
    Serial.println(F("    on, off, toggle"));
    Serial.println(F("  childlock <cmd> - Control child lock"));
    Serial.println(F("    engage, release, status"));
    Serial.println(F(""));
    Serial.println(F("Testing & Maintenance:"));
    Serial.println(F("  t, test     - Test keypad"));
    Serial.println(F("  b, buzzer   - Test buzzer"));
    Serial.println(F("  sensors     - Show sensor status"));
    Serial.println(F("  password    - Password management"));
    Serial.println(F(""));
    Serial.println(F("System Management:"));
    Serial.println(F("  factoryreset - Reset to factory defaults"));
    Serial.println(F("  rl, resetlight - Reset light override"));
    Serial.println(F("  ls, lightstatus - Show light status"));
    Serial.println(F(""));
}

void SerialCommandTask::printTaskStats()
{
    uint8_t taskCount = OS.getTaskCount();
    Serial.print(F("=== Task Stats ===\n"));
    Serial.print(F("Tasks: "));
    Serial.println(taskCount);

    for (uint8_t i = 1; i <= taskCount; i++)  // Task IDs start from 1, not 0
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
    Serial.println(F("  Yellow Button: Active"));
    Serial.println(F("  Keypad: 4-digit membrane"));
    Serial.println(F("  Red LED: Status indicator"));
    Serial.println(F("  Green LED: Status indicator"));
    Serial.println(F("  Light Control: PWM controlled"));
    Serial.println(F("  MB Light Sensor: Active"));
    Serial.println(F("  Device Running Sensor: Active"));
    Serial.println(F("  Child Lock Power: Active"));
    Serial.println(F("  Child Lock Screen: Active"));
    Serial.println(F("Features: Watchdog, Reset tracking, Task monitoring, Smart child lock"));
}

void SerialCommandTask::handleLEDCommand(const char *args)
{
    while (*args == ' ')
    {
        args++;
    }
    if (strcasecmp_P(args, PSTR("locked")) == 0)
    {
        logInfo(F("LED LOCKED"));
        publish(EVT_LED_LOCKED, 0, 0);
    }
    else if (strcasecmp_P(args, PSTR("unlocked")) == 0)
    {
        logInfo(F("LED UNLOCKED"));
        publish(EVT_LED_UNLOCKED, 0, 0);
    }
    else if (strcasecmp_P(args, PSTR("to_be_locked")) == 0)
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
    if (strcasecmp_P(args, PSTR("on")) == 0)
    {
        logInfof(F("Light %S"), F("ON"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 1);
    }
    else if (strcasecmp_P(args, PSTR("off")) == 0)
    {
        logInfof(F("Light %S"), F("OFF"));
        publish(TOPIC_LIGHT_EVENTS, EVT_LIGHT_TOGGLE, 0);
    }
    else if (strcasecmp_P(args, PSTR("toggle")) == 0)
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
    if (strcasecmp_P(args, PSTR("engage")) == 0)
    {
        logInfo(F("Engaging child lock"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_ENGAGE, 0);
    }
    else if (strcasecmp_P(args, PSTR("release")) == 0)
    {
        logInfo(F("Releasing child lock"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_RELEASE, 0);
    }
    else if (strcasecmp_P(args, PSTR("reset")) == 0)
    {
        logInfo(F("Resetting child lock timeout"));
        publish(TOPIC_CHILD_LOCK_EVENTS, EVT_CHILD_LOCK_TIMEOUT_RESET, 0);
    }
    else if (strcasecmp_P(args, PSTR("status")) == 0)
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
    if (strcasecmp_P(args, PSTR("show")) == 0)
    {
        Serial.println(F("Password cannot be shown for security. Use 'password reload' or keypad change mode."));
    }
    else if (strcasecmp_P(args, PSTR("reload")) == 0)
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

    Serial.print(F("Motherboard Light Sensor: "));
    Serial.println(mbLightSensor ? F("HIGH") : F("LOW"));

    Serial.print(F("Device Running Sensor: "));
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
    Serial.println(F("=== Memory Info ==="));

    // Basic RAM info using Arduino functions
    Serial.println(F("\nRAM Usage:"));
    Serial.print(F("  Free RAM: "));
    Serial.print(freeMemory());
    Serial.println(F(" bytes"));

    // Calculate used RAM (approximate)
    uint16_t free_ram = freeMemory();
    uint16_t total_ram = 2048; // Arduino Nano has 2KB RAM
    uint16_t used_ram = total_ram - free_ram;

    Serial.print(F("  Used RAM: "));
    Serial.print(used_ram);
    Serial.println(F(" bytes"));

    Serial.print(F("  Total RAM: "));
    Serial.print(total_ram);
    Serial.println(F(" bytes"));

    Serial.print(F("  Usage: "));
    Serial.print((uint32_t)used_ram * 100UL / (uint32_t)total_ram);
    Serial.println(F("%"));

    // Heap allocation info using FsmOS memory leak detection
    MemoryStats leak_stats;
    if (OS.getMemoryLeakStats(leak_stats))
    {
        Serial.println(F("\nHeap Allocation:"));
        Serial.print(F("  Allocated: "));
        Serial.print(leak_stats.total_allocated);
        Serial.println(F(" bytes"));

        Serial.print(F("  Freed:     "));
        Serial.print(leak_stats.total_freed);
        Serial.println(F(" bytes"));

        Serial.print(F("  Current:   "));
        Serial.print(leak_stats.current_usage);
        Serial.println(F(" bytes"));

        Serial.print(F("  Peak:      "));
        Serial.print(leak_stats.peak_usage);
        Serial.println(F(" bytes"));

        // Calculate heap free space
        uint16_t heap_total = 512; // Approximate heap size
        uint16_t heap_free = heap_total - leak_stats.current_usage;

        Serial.print(F("  Heap Free: "));
        Serial.print(heap_free);
        Serial.println(F(" bytes"));

        Serial.print(F("  Heap Usage: "));
        Serial.print((uint32_t)leak_stats.current_usage * 100UL / heap_total);
        Serial.println(F("%"));
    }
    else
    {
        Serial.println(F("\nHeap Allocation: Not available"));
    }

    // Flash usage
    Serial.println(F("\nFlash Usage:"));
    Serial.print(F("  Program size: "));
    Serial.print(getProgramSize());
    Serial.println(F(" bytes"));

    Serial.print(F("  Available: "));
    Serial.print(30720 - getProgramSize()); // 30KB flash
    Serial.println(F(" bytes"));

    Serial.print(F("  Usage: "));
    Serial.print((uint32_t)getProgramSize() * 100UL / 30720UL);
    Serial.println(F("%"));

    // EEPROM usage - calculate actual usage from EEPROM
    Serial.println(F("\nEEPROM Usage:"));

    // Get actual EEPROM usage by reading from EEPROM
    uint16_t eeprom_used = getActualEEPROMUsage();
    uint16_t eeprom_total = 1024; // Arduino Nano has 1KB EEPROM
    uint16_t eeprom_free = eeprom_total - eeprom_used;

    Serial.print(F("  Used:  "));
    Serial.print(eeprom_used);
    Serial.println(F(" bytes"));

    Serial.print(F("  Free:  "));
    Serial.print(eeprom_free);
    Serial.println(F(" bytes"));

    Serial.print(F("  Usage: "));
    Serial.print((uint32_t)eeprom_used * 100UL / eeprom_total);
    Serial.println(F("%"));

    Serial.println(F("  Layout:"));
    Serial.println(F("    Address 0:   Light brightness (1 byte)"));
    Serial.println(F("    Address 1:   Light state (1 byte)"));
    Serial.println(F("    Address 16:  Password magic (1 byte)"));
    Serial.println(F("    Address 17-20: Password (4 bytes)"));

    Serial.println(F("\n"));

    // Task information with RAM usage
    Serial.println(F("=== Task Information ==="));
    uint8_t taskCount = OS.getTaskCount();
    Serial.print(F("  Total Tasks: "));
    Serial.println(taskCount);

    uint16_t totalTaskMemory = 0;
    uint8_t printedTasks = 0;
    for (uint8_t i = 1; i <= taskCount; i++)  // Task IDs start from 1, not 0
    {
        TaskStats stats;
        TaskMemoryInfo memInfo;
        if (OS.getTaskStats(i, stats) && OS.getTaskMemoryInfo(i, memInfo))
        {
            Task *task = OS.getTask(i);
            if (task)
            {
                Serial.print(F("  Task "));
                Serial.print(i);
                Serial.print(F(": "));
                Serial.print(Task::readTaskName(task));
                Serial.print(F(" (RAM: "));
                Serial.print(memInfo.total_allocated);
                Serial.print(F("B, Runs: "));
                Serial.print(stats.runCount);
                Serial.print(F(", Max: "));
                Serial.print(stats.maxExecTimeUs);
                Serial.println(F("us)"));

                totalTaskMemory += memInfo.total_allocated;
                printedTasks++;
            }
        }
        else
        {
            // Task exists but stats/memory info failed
            Serial.print(F("  Task "));
            Serial.print(i);
            Serial.println(F(": <stats unavailable>"));
            printedTasks++;
        }
    }

    Serial.print(F("  Printed Tasks: "));
    Serial.print(printedTasks);
    Serial.print(F("/"));
    Serial.println(taskCount);
    Serial.print(F("  Total Task RAM: "));
    Serial.print(totalTaskMemory);
    Serial.println(F(" bytes"));

    Serial.println(F("\n"));
}

void SerialCommandTask::printUnknownCommand(const char *command)
{
    Serial.print(F("Unknown command: '"));
    Serial.print(command);
    Serial.println(F("'"));
    Serial.println(F("Type 'help' for available commands"));
}
