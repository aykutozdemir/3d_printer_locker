#pragma once

// Hardware pin definitions
#define YELLOW_BUTTON_PIN 6

// Keypad pin definitions
#define KEYPAD_PIN_1 2  // D2
#define KEYPAD_PIN_2 3  // D3
#define KEYPAD_PIN_3 4  // D4
#define KEYPAD_PIN_4 5  // D5

// Status LED pin definitions
#define RED_LED_PIN A1
#define GREEN_LED_PIN A2

// Buzzer pin definition
#define BUZZER_PIN A3

// Light control pin definition
#define LIGHT_PIN 10  // D10

// Motherboard light sensor pin definition
#define MB_LIGHT_SENSOR_PIN 7  // D7

// Device running sensor pin definition
#define DEVICE_RUNNING_SENSOR_PIN A4  // A4

// Door control pin definitions
#define FRONT_DOOR_PIN 11  // D11
#define TOP_DOOR_PIN 12    // D12

// Child lock control pin definitions
#define CHILD_LOCK_POWER_PIN 13  // D13 - Power button lock
#define CHILD_LOCK_SCREEN_PIN A0 // A0 - Touchscreen lock

// Door sensor pin definitions
#define FRONT_DOOR_SENSOR_PIN 8   // D8 - GND when door closed
#define TOP_DOOR_SENSOR_PIN 9     // D9 - GND when door opened

// Event message types
#define EVT_BUTTON_SHORT_CLICK 1
#define EVT_BUTTON_LONG_CLICK  2

// Message topics
#define TOPIC_BUTTON_EVENTS 1
#define TOPIC_LIGHT_EVENTS 2
#define TOPIC_KEYPAD_EVENTS 3
#define TOPIC_STATUS_LED_EVENTS 4
#define TOPIC_PASSWORD_EVENTS 5
#define TOPIC_DOOR_EVENTS 6
#define TOPIC_DOOR_SENSOR_EVENTS 7
#define TOPIC_BUZZER_EVENTS 8
#define TOPIC_CHILD_LOCK_EVENTS 9
#define TOPIC_MB_LIGHT_SENSOR_EVENTS 10
#define TOPIC_DEVICE_RUNNING_EVENTS 11

// Keypad event types
#define EVT_KEYPAD_1_PRESSED 10
#define EVT_KEYPAD_2_PRESSED 11
#define EVT_KEYPAD_3_PRESSED 12
#define EVT_KEYPAD_4_PRESSED 13
// Keypad long-press event types
#define EVT_KEYPAD_1_LONG_PRESSED 14
#define EVT_KEYPAD_2_LONG_PRESSED 15
#define EVT_KEYPAD_3_LONG_PRESSED 16
#define EVT_KEYPAD_4_LONG_PRESSED 17

// Status LED event types
#define EVT_LED_LOCKED 20
#define EVT_LED_UNLOCKED 21
#define EVT_LED_TO_BE_LOCKED 22
#define EVT_LED_TO_BE_OPENED 23
#define EVT_LED_CHILD_UNLOCKED 24

// Light control event types
#define EVT_LIGHT_TOGGLE 30
#define EVT_LIGHT_DIM_START 31
#define EVT_LIGHT_DIM_STOP 32
#define EVT_MB_LIGHT_SENSOR_CHANGED 33
#define EVT_DEVICE_RUNNING_CHANGED 34

// Password and door control event types
#define EVT_PASSWORD_DIGIT_ENTERED 40
#define EVT_PASSWORD_CORRECT 41
#define EVT_PASSWORD_WRONG 42
#define EVT_PASSWORD_TIMEOUT 43
#define EVT_PASSWORD_RELOAD_REQUEST 44
#define EVT_PASSWORD_SET_FACTORY 45
#define EVT_DOOR_TOP_RELEASE 50
#define EVT_DOOR_FRONT_RELEASE 51
#define EVT_DOOR_BOTH_RELEASE 52
#define EVT_DOOR_FRONT_OPENED 60
#define EVT_DOOR_TOP_OPENED 61
#define EVT_DOOR_FRONT_CLOSED 62
#define EVT_DOOR_TOP_CLOSED 63

// Buzzer event types
#define EVT_BUZZER_BUTTON_PRESS 70
#define EVT_BUZZER_KEYPAD_PRESS 71
#define EVT_BUZZER_WRONG_PASSWORD 72
#define EVT_BUZZER_CORRECT_PASSWORD 73
#define EVT_BUZZER_DOOR_RELEASED 74
#define EVT_BUZZER_DOOR_CLOSED 75
#define EVT_BUZZER_ANGRY_SOUND 76
#define EVT_BUZZER_ANGRY_SOUND_START 77
#define EVT_BUZZER_ANGRY_SOUND_STOP 78
#define EVT_BUZZER_TOP_DOOR_SELECTED 79
#define EVT_BUZZER_FRONT_DOOR_SELECTED 80
#define EVT_BUZZER_BOTH_DOORS_SELECTED 81
#define EVT_BUZZER_CHILD_LOCK_SELECTED 82

// Child lock event types
#define EVT_CHILD_LOCK_RELEASE 83
#define EVT_CHILD_LOCK_ENGAGE 84
#define EVT_CHILD_LOCK_TIMEOUT_RESET 85

// Button timing constants
#define DEBOUNCE_TIME_MS 50
#define LONG_PRESS_TIME_MS 1000

// Password timing constants
#define PASSWORD_DIGIT_TIMEOUT_MS 3000  // 3 seconds per digit
#define PASSWORD_LENGTH 4
#define DEFAULT_PASSWORD "1234"

// Child lock timing
#define CHILD_LOCK_TIMEOUT_MS 60000  // 1 minute auto re-engage

// EEPROM layout (avoid address 0 used by light brightness)
#define EEPROM_PASSWORD_MAGIC_ADDR 16
#define EEPROM_PASSWORD_ADDR       17  // 17..20 inclusive for 4-digit PIN
#define EEPROM_PASSWORD_MAGIC_VAL  0xA5
// Light on/off state address (brightness at 0, state at 1)
#define EEPROM_LIGHT_STATE_ADDR    1
