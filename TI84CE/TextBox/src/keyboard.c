#include <tice.h>
#include <keypadc.h>
#include <string.h>
#include <stdlib.h>
#include "headers/keyboard_private.h"

#include "headers/log.h"

// Maximum number of key callbacks that can be registered
#define MAX_CALLBACKS 16

// Static state variables
static CallbackEntry callbacks[MAX_CALLBACKS];   /**< Array of callback entries */
static int next_callback_id = 1;                 /**< Next available callback ID (starts at 1) */
static bool initialized = false;                 /**< Whether the keyboard handler has been initialized */

// Default configuration for key handling
static int key_sensitivity = 50;     /**< Sensitivity for key polling in milliseconds */
static int key_repeat_delay = 500;   /**< Initial delay before key repeat in milliseconds */
static int key_repeat_interval = 100; /**< Interval between repeated keys in milliseconds */

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
    
    next_callback_id = 1;
    initialized = true;
    
    log_message("Keyboard subsystem initialized.");
}

/**
 * Register a callback for key down events.
 * 
 * @param callback The function to call when a key is pressed down.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int key_register_down(void* obj, KeyDownCallback callback) {
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
    callbacks[slot].obj = obj;
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
int key_register_press(void* obj, KeyPressCallback callback, int repeat_delay_ms, int repeat_interval_ms) {
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
    callbacks[slot].obj = obj;
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
int key_register_up(void* obj, KeyUpCallback callback) {
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
    callbacks[slot].obj = obj;
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].callback.up = callback;
    callbacks[slot].type = CB_UP;

    log_message("Key up callback registered successfully.");
    
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
 * Configure key processing parameters.
 * 
 * @param sensitivity polling sensitivity in milliseconds
 * @param repeat_delay time before key repeat starts in milliseconds
 * @param repeat_interval interval between repeated keys in milliseconds
 */
void key_configure(int sensitivity, int repeat_delay, int repeat_interval) {
    if (sensitivity > 0) key_sensitivity = sensitivity;
    if (repeat_delay > 0) key_repeat_delay = repeat_delay;
    if (repeat_interval > 0) key_repeat_interval = repeat_interval;
    
    log_message("Key configuration updated: sensitivity=%d, repeat_delay=%d, repeat_interval=%d", 
                key_sensitivity, key_repeat_delay, key_repeat_interval);
}

/**
 * Get the current time in milliseconds.
 * 
 * @return The current time in milliseconds.
 */
static unsigned long key_get_millis(void) {
    // On the TI-84 CE, timer_1_Counter runs at 32768 Hz
    // Convert to milliseconds (approximately)
    return (unsigned long)(timer_1_Counter / 32.768);
}

/**
 * Wait for a key press and process it completely.
 * This is a blocking function that handles the entire key lifecycle:
 * - Waits for a key to be pressed
 * - Processes down callbacks
 * - Handles repeat logic and callbacks
 * - Processes up callbacks when the key is released
 * 
 * @return The key that was pressed and released.
 */
Key key_wait(void) {
    if (!initialized) key_init();
    log_message("Waiting for key...");
    log_message("key_sensitivity: %d", key_sensitivity);
    log_message("key_repeat_delay: %d", key_repeat_delay);
    log_message("key_repeat_interval: %d", key_repeat_interval);
    
    Key key = KEY_NONE;
    bool key_down = false;
    
    // Wait for a key press
    while (!key_down) {
        kb_Scan();
        
        // Check each key group
        for (int group = 1; group <= 7; group++) {
            uint8_t group_state = kb_Data[group];
            if (!group_state) continue;
            
            // A key is pressed in this group, check which one
            for (int bit = 0; bit < 8; bit++) {
                uint8_t mask = 1 << bit;
                if (!(group_state & mask)) continue;
                
                // Found the pressed key
                key_down = true;
                key = (Key)((group << 8) | mask);
                
                // Process key down callbacks
                for (int i = 0; i < MAX_CALLBACKS; i++) {
                    if (callbacks[i].active && callbacks[i].type == CB_DOWN) {
                        callbacks[i].callback.down(callbacks[i].obj, key);
                    }
                }
                break;
            }
            if (key_down) break;
        }
        
        // If no key is pressed yet, delay a bit to reduce CPU usage
        if (!key_down) {
            delay(key_sensitivity);
        }
    }
    
    // Key is now pressed, handle repeat logic
    unsigned long start_time = key_get_millis();
    unsigned long last_repeat_time = start_time;
    int wait_delay = -1;
    
    // Process repeats while key is held down
    do {
        unsigned long current_time = key_get_millis();
        unsigned long elapsed = current_time - last_repeat_time;
        
        // Check if it's time for a repeat
        log_message("wait_delay: %d elapsed: %lu", wait_delay, elapsed);
        if ((int)elapsed >= wait_delay) {
            log_message("Processing callbacks");
            
            // Process press callbacks
            for (int i = 0; i < MAX_CALLBACKS; i++) {
                if (callbacks[i].active && callbacks[i].type == CB_PRESS) {
                    callbacks[i].callback.press(callbacks[i].obj, key);
                }
            }
            
            // Update timing for next repeat
            wait_delay = (wait_delay == -1) ? key_repeat_delay : key_repeat_interval;
            last_repeat_time = current_time;
        }
        log_message("wait_delay: %d ", wait_delay);
        
        // Small delay to reduce CPU usage
        delay(key_sensitivity);
        
        // Check if key is still pressed
        kb_Scan();
        uint8_t group = KEY_GROUP(key);
        uint8_t mask = KEY_MASK(key);
        key_down = (kb_Data[group] & mask) != 0;        
    } while (key_down);
    
    // Key has been released, process up callbacks
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (callbacks[i].active && callbacks[i].type == CB_UP) {
            callbacks[i].callback.up(callbacks[i].obj, key);
        }
    }
    
    log_message("Key processed: %d", key);
    return key;
}

/**
 * Check if a specific key is currently pressed.
 * Note: This does not process callbacks, it just checks the key state.
 * 
 * @param key The key to check.
 * @return True if the key is pressed, false otherwise.
 */
bool key_is_pressed(Key key) {
    // Scan the keyboard
    kb_Scan();
    
    int group = KEY_GROUP(key);
    int mask = KEY_MASK(key);
    return (kb_Data[group] & mask) ? true : false;
}

/**
 * Wait for any key to be pressed and released.
 * This is a simple wrapper around key_wait.
 * 
 * @return The key that was pressed and released.
 */
Key key_wait_any(void) {
    return key_wait();
}
