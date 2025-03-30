#include <tice.h>
#include <keypadc.h>
#include <string.h>
#include <stdlib.h>
#include "headers/keyboard_private.h"

#include "headers/log.h"

// Maximum number of key callbacks that can be registered
#define MAX_CALLBACKS 16

/**
 * Internal structure for callback registration.
 */
typedef struct {
    bool active;                 /**< Whether this entry is active */
    int id;                      /**< Unique ID for this callback */
    union {
        KeyDownCallback down;    /**< Function pointer for down callback */
        KeyPressCallback press;  /**< Function pointer for press callback */
        KeyUpCallback up;        /**< Function pointer for up callback */
        KeyHoldCallback hold;    /**< Function pointer for hold callback */
    } callback;
    int type;                    /**< Type of callback (0=down, 1=press, 2=up, 3=hold) */
    int repeat_delay;            /**< Time before repeating (for press) */
    int repeat_interval;         /**< Interval between repeats (for press) */
    int hold_time;               /**< Time before triggering hold event (for hold) */
    bool hold_repeat;            /**< Whether hold event should repeat */
} CallbackEntry;

/**
 * Callback types enumeration.
 */
enum {
    CB_DOWN = 0, /**< Key down event */
    CB_PRESS = 1, /**< Key press event */
    CB_UP = 2, /**< Key up event */
    CB_HOLD = 3 /**< Key hold event */
};

// Static state variables
static CallbackEntry callbacks[MAX_CALLBACKS];   /**< Array of callback entries */
static int next_callback_id = 1;                 /**< Next available callback ID (starts at 1) */
static bool initialized = false;                 /**< Whether the keyboard handler has been initialized */
static Key last_key_pressed = KEY_NONE;          /**< The last key that was pressed */
static Key current_key = KEY_NONE;               /**< The current key that is pressed */
static bool key_states[8][8];                    /**< Track state of each key for edge detection */
static unsigned long key_press_times[8][8];      /**< Track when each key was pressed */
static unsigned long key_last_repeat[8][8];      /**< Track when each key last triggered a repeat */

/**
 * Initialize the keyboard subsystem.
 */
void key_init(void) {
    if (initialized) {
        log_message("Keyboard subsystem already initialized.");
        return;
    }

    log_message("Initializing keyboard subsystem...");
    // Clear all callback entries
    memset(callbacks, 0, sizeof(callbacks));
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        callbacks[i].active = false;
    }
    
    // Clear key states and timing
    memset(key_states, 0, sizeof(key_states));
    memset(key_press_times, 0, sizeof(key_press_times));
    memset(key_last_repeat, 0, sizeof(key_last_repeat));
    
    next_callback_id = 1;
    last_key_pressed = KEY_NONE;
    current_key = KEY_NONE;
    initialized = true;
    log_message("Keyboard subsystem initialized.");
}

/**
 * Register a callback for key down events.
 * 
 * @param callback The function to call when a key is pressed down.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int key_register_down(KeyDownCallback callback) {
    if (!initialized) key_init();

    log_message("Registering key down callback...");
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("Failed to register key down callback: No available slots.");
        return -1;
    }
    
    // Set up the callback entry
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].callback.down = callback;
    callbacks[slot].type = CB_DOWN;

    log_message("Key down callback registered successfully.");
    
    return callbacks[slot].id;
}

/**
 * Register a callback for key press events.
 * 
 * @param callback The function to call when a key is pressed.
 * @param repeat_delay_ms Delay before repeating the press event, in milliseconds.
 * @param repeat_interval_ms Interval between repeated press events, in milliseconds.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int key_register_press(KeyPressCallback callback, int repeat_delay_ms, int repeat_interval_ms) {
    if (!initialized) key_init();

    log_message("Registering key press callback...");
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("Failed to register key press callback: No available slots.");
        return -1;
    }
    
    // Set up the callback entry
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].callback.press = callback;
    callbacks[slot].type = CB_PRESS;
    callbacks[slot].repeat_delay = repeat_delay_ms;
    callbacks[slot].repeat_interval = repeat_interval_ms;

    log_message("Key press callback registered successfully.");
    
    return callbacks[slot].id;
}

/**
 * Register a callback for key up events.
 * 
 * @param callback The function to call when a key is released.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int key_register_up(KeyUpCallback callback) {
    if (!initialized) key_init();

    log_message("Registering key up callback...");
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("Failed to register key up callback: No available slots.");
        return -1;
    }
    
    // Set up the callback entry
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].callback.up = callback;
    callbacks[slot].type = CB_UP;

    log_message("Key up callback registered successfully.");
    
    return callbacks[slot].id;
}

/**
 * Register a callback for key hold events.
 * 
 * @param callback The function to call when a key is held.
 * @param hold_time_ms Time in milliseconds before the hold event is triggered.
 * @param repeat Whether the hold event should repeat.
 * @param repeat_interval_ms Interval between repeated hold events, in milliseconds.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int key_register_hold(KeyHoldCallback callback, int hold_time_ms, bool repeat, int repeat_interval_ms) {
    if (!initialized) key_init();

    log_message("Registering key hold callback...");
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("Failed to register key hold callback: No available slots.");
        return -1;
    }
    
    // Set up the callback entry
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].callback.hold = callback;
    callbacks[slot].type = CB_HOLD;
    callbacks[slot].hold_time = hold_time_ms;
    callbacks[slot].hold_repeat = repeat;
    callbacks[slot].repeat_interval = repeat_interval_ms;

    log_message("Key hold callback registered successfully.");
    
    return callbacks[slot].id;
}

/**
 * Unregister a callback by its ID.
 * 
 * @param callback_id The ID of the callback to unregister.
 * @return True if the callback was successfully unregistered, false otherwise.
 */
bool key_unregister(int callback_id) {
    if (!initialized) return false;

    log_message("Unregistering callback with ID %d...", callback_id);
    
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (callbacks[i].active && callbacks[i].id == callback_id) {
            callbacks[i].active = false;
            log_message("Callback with ID %d unregistered successfully.", callback_id);
            return true;
        }
    }

    log_message("Failed to unregister callback with ID %d: Not found.", callback_id);
    
    return false;
}

/**
 * Clear all registered callbacks.
 */
void key_clear_callbacks(void) {
    if (!initialized) return;

    log_message("Clearing all registered callbacks...");
    
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        callbacks[i].active = false;
    }

    log_message("All callbacks cleared.");
}

/**
 * Check if a specific key is currently pressed.
 * 
 * @param key The key to check.
 * @return True if the key is pressed, false otherwise.
 */
bool key_is_pressed(Key key) {
    int group = KEY_GROUP(key);
    int mask = KEY_MASK(key);
    return (kb_Data[group] & mask) ? true : false;
}

/**
 * Get the currently pressed key, or KEY_NONE if no key is pressed.
 * 
 * @return The currently pressed key.
 */
Key key_get_pressed(void) {
    // Scan the keyboard
    kb_Scan();
    
    // Look for a pressed key
    for (int group = 1; group <= 7; group++) {
        uint8_t group_state = kb_Data[group];
        
        if (group_state) {
            // Find the specific key bit
            for (int bit = 0; bit < 8; bit++) {
                uint8_t mask = 1 << bit;
                if (group_state & mask) {
                    // Return the first key found
                    return (Key)((group << 8) | mask);
                }
            }
        }
    }
    
    return KEY_NONE;
}

/**
 * Process keyboard events once and trigger callbacks.
 */
void key_scan(void) {
    if (!initialized) key_init();

    log_message("Scanning keyboard...");
    
    // Get current time for timing logic
    unsigned long current_time = key_get_millis();
    
    // Scan keyboard
    kb_Scan();
    
    // Reset current key
    current_key = KEY_NONE;
    
    // Process each key group
    for (int group = 1; group <= 7; group++) {
        uint8_t group_state = kb_Data[group];
        
        // Check each bit in the group
        for (int bit = 0; bit < 8; bit++) {
            uint8_t mask = 1 << bit;
            bool is_pressed = (group_state & mask) != 0;
            
            // If this key is pressed, update current_key
            if (is_pressed && current_key == KEY_NONE) {
                current_key = (Key)((group << 8) | mask);
                log_message("Key pressed: %d", current_key);
            }
            
            // If the key state changed, trigger appropriate callbacks
            if (is_pressed != key_states[group][bit]) {
                Key key = (Key)((group << 8) | mask);
                
                if (is_pressed) {
                    // Key was just pressed
                    key_press_times[group][bit] = current_time;
                    key_last_repeat[group][bit] = 0;
                    last_key_pressed = key;
                    
                    // Trigger down callbacks
                    for (int i = 0; i < MAX_CALLBACKS; i++) {
                        if (callbacks[i].active && callbacks[i].type == CB_DOWN) {
                            callbacks[i].callback.down(key);
                        }
                    }
                } else {
                    // Key was just released
                    // Trigger up callbacks
                    for (int i = 0; i < MAX_CALLBACKS; i++) {
                        if (callbacks[i].active && callbacks[i].type == CB_UP) {
                            callbacks[i].callback.up(key);
                        }
                    }
                }
                
                // Update state
                key_states[group][bit] = is_pressed;
            }
            
            // Handle press and hold callbacks for pressed keys
            if (is_pressed) {
                unsigned long press_duration = current_time - key_press_times[group][bit];
                Key key = (Key)((group << 8) | mask);
                
                // Process press callbacks (with repeat)
                for (int i = 0; i < MAX_CALLBACKS; i++) {
                    if (callbacks[i].active && callbacks[i].type == CB_PRESS) {
                        // Initial press or repeat timing
                        if (key_last_repeat[group][bit] == 0) {
                            // First press
                            callbacks[i].callback.press(key);
                            key_last_repeat[group][bit] = current_time;
                        } else if (press_duration >= (unsigned long)callbacks[i].repeat_delay &&
                                  (current_time - key_last_repeat[group][bit]) >= 
                                   (unsigned long)callbacks[i].repeat_interval) {
                            // Repeating press
                            callbacks[i].callback.press(key);
                            key_last_repeat[group][bit] = current_time;
                        }
                    }
                }
                
                // Process hold callbacks
                for (int i = 0; i < MAX_CALLBACKS; i++) {
                    if (callbacks[i].active && callbacks[i].type == CB_HOLD) {
                        // Check if hold threshold has been met
                        if (press_duration >= (unsigned long)callbacks[i].hold_time) {
                            // First hold or repeating hold
                            if (key_last_repeat[group][bit] == 0 || 
                                (callbacks[i].hold_repeat && 
                                 (current_time - key_last_repeat[group][bit]) >= 
                                  (unsigned long)callbacks[i].repeat_interval)) {
                                
                                callbacks[i].callback.hold(key, (int)press_duration);
                                key_last_repeat[group][bit] = current_time;
                            }
                        }
                    }
                }
            }
        }
    }

    log_message("Keyboard scan complete.");
}

/**
 * Wait for any key to be pressed and released.
 * 
 * @return The key that was pressed and released.
 */
Key key_wait_any(void) {
    log_message("Waiting for any key press...");
    Key pressed_key = KEY_NONE;
    
    // Wait for a key press
    while (pressed_key == KEY_NONE) {
        kb_Scan();
        pressed_key = key_get_pressed();
        delay(10);
    }
    
    // Remember which key was pressed
    Key result = pressed_key;
    
    // Wait for key release
    while (key_get_pressed() != KEY_NONE) {
        kb_Scan();
        delay(10);
    }

    log_message("Key pressed and released: %d", result);
    
    return result;
}

/**
 * Wait until all keys are released.
 */
void key_wait_release(void) {
    log_message("Waiting for all keys to be released...");
    while (key_get_pressed() != KEY_NONE) {
        kb_Scan();
        delay(10);
    }
    log_message("All keys released.");
}

/**
 * Get the current time in milliseconds.
 * 
 * @return The current time in milliseconds.
 */
unsigned long key_get_millis(void) {
    // On the TI-84 CE, timer_1_Counter runs at 32768 Hz
    // Convert to milliseconds (approximately)
    unsigned long millis = (unsigned long)(timer_1_Counter / 32.768);
    log_message("Current time in milliseconds: %lu", millis);
    return millis;
}
