#include <tice.h>
#include <keypadc.h>
#include <string.h>
#include "headers/kb_handler.h"
#include "headers/kb_handler_private.h"

/**
 * Structure representing a callback entry.
 */
typedef struct {
    bool active;                    /**< Whether this entry is active. */
    KBCallbackID id;                /**< Unique ID for this callback. */
    CombinedKey key;                /**< The key associated with this callback. */
    CallbackType type;              /**< The type of callback (press, release, hold). */
    union {
        KBPressCallback press;      /**< Function pointer for press callback. */
        KBReleaseCallback release;  /**< Function pointer for release callback. */
        KBHoldCallback hold;        /**< Function pointer for hold callback. */
        void (*any_press)(CombinedKey key); /**< Function pointer for any key press callback. */
    } callback;
    bool was_pressed;               /**< Previous state of the key for edge detection. */
    unsigned long press_time;       /**< Time when the key was first pressed (for hold detection). */
    int hold_trigger_time;          /**< Time to wait before triggering a hold event (in ms). */
    bool hold_repeat;               /**< Whether the hold event should repeat. */
    int hold_interval;              /**< Interval between hold events when repeating (in ms). */
    unsigned long last_hold_time;   /**< Time of the last hold event. */
    bool is_any_key;                /**< Whether this is a callback for any key. */
} CallbackEntry;

// Static state variables
static CallbackEntry callbacks[KB_MAX_CALLBACKS];   /**< Array of callback entries. */
static int next_callback_id = 1;                    /**< Next available callback ID (starts at 1). */
static bool initialized = false;                    /**< Whether the keyboard handler has been initialized. */
static CombinedKey last_key_pressed = 0;            /**< The last key that was pressed. */

/**
 * Checks if a specific key is pressed
 * 
 * @param combined_key One of the KEY_* constants
 * @return true if the key is pressed, false otherwise
 */
bool kb_is_key_pressed(CombinedKey combined_key) {
    int group = KEY_GROUP(combined_key);
    int mask = KEY_MASK(combined_key);
    return (kb_Data[group] & mask) ? true : false;
}

/**
 * Initializes the keyboard handler.
 */
void kb_init(void) {
    if (initialized) return;

    // Clear all callback entries
    memset(callbacks, 0, sizeof(callbacks));
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        callbacks[i].active = false;
    }

    next_callback_id = 1;
    last_key_pressed = 0;
    initialized = true;
}

/**
 * Registers a callback for key press events.
 * 
 * @param key The key to associate with the callback.
 * @param callback The function to call when the key is pressed.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
KBCallbackID kb_register_press(CombinedKey key, KBPressCallback callback) {
    return register_callback(key, CB_PRESS, (void*)callback, 0, false, 0);
}

/**
 * Registers a callback for key release events.
 * 
 * @param key The key to associate with the callback.
 * @param callback The function to call when the key is released.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
KBCallbackID kb_register_release(CombinedKey key, KBReleaseCallback callback) {
    return register_callback(key, CB_RELEASE, (void*)callback, 0, false, 0);
}

/**
 * Registers a callback for key hold events.
 * 
 * @param key The key to associate with the callback.
 * @param callback The function to call when the key is held.
 * @param trigger_time_ms The time to wait before triggering the hold event (in ms).
 * @param repeat Whether the hold event should repeat.
 * @param repeat_interval_ms The interval between hold events when repeating (in ms).
 * @return The ID of the registered callback, or -1 if registration failed.
 */
KBCallbackID kb_register_hold(CombinedKey key, KBHoldCallback callback, 
                              int trigger_time_ms, bool repeat, int repeat_interval_ms) {
    return register_callback(key, CB_HOLD, (void*)callback, 
                             trigger_time_ms, repeat, repeat_interval_ms);
}

/**
 * Registers a callback for any key press.
 * 
 * @param callback The function to call when any key is pressed.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
KBCallbackID kb_register_any_press(void (*callback)(CombinedKey key)) {
    if (!initialized) kb_init();

    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) {
            slot = i;
            break;
        }
    }

    // If no slot available, return error
    if (slot < 0) return -1;

    // Set up the callback entry
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].key = 0; // Not used for any-key callbacks
    callbacks[slot].type = CB_PRESS;
    callbacks[slot].is_any_key = true;
    callbacks[slot].callback.any_press = callback;
    callbacks[slot].was_pressed = false;

    return callbacks[slot].id;
}

/**
 * Unregisters a callback by its ID.
 * 
 * @param callback_id The ID of the callback to unregister.
 * @return True if the callback was successfully unregistered, false otherwise.
 */
bool kb_unregister_id(KBCallbackID callback_id) {
    if (!initialized) kb_init();

    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (callbacks[i].active && callbacks[i].id == callback_id) {
            callbacks[i].active = false;
            return true;
        }
    }

    return false;
}

/**
 * Unregisters all callbacks associated with a specific key.
 * 
 * @param key The key whose callbacks should be unregistered.
 * @return The number of callbacks that were unregistered.
 */
int kb_unregister(CombinedKey key) {
    if (!initialized) kb_init();

    int count = 0;
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (callbacks[i].active && !callbacks[i].is_any_key && callbacks[i].key == key) {
            callbacks[i].active = false;
            count++;
        }
    }

    return count;
}

/**
 * Clears all registered callbacks.
 */
void kb_clear(void) {
    if (!initialized) return;

    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        callbacks[i].active = false;
    }
    debounce();
}

/**
 * Gets the last key that was pressed.
 * 
 * @return The last key that was pressed, or 0 if no key has been pressed.
 */
CombinedKey kb_get_last_key(void) {
    return last_key_pressed;
}

/**
 * Clears the last key pressed.
 */
void kb_clear_last_key(void) {
    last_key_pressed = 0;
}

/**
 * Processes keyboard events and triggers callbacks as necessary.
 */
void kb_process(void) {
    if (!initialized) kb_init();

    // Get current time for hold timing
    unsigned long current_time = get_millis();

    // Scan keyboard once
    kb_Scan();

    // Check for any keys being pressed for the first time
    // Iterate through all possible keys to check if any were just pressed
    for (int group = 1; group <= 7; group++) {
        // Get the current and previous state of this group
        uint8_t group_state = kb_Data[group];
        
        // Skip if no keys in this group are pressed
        if (!group_state) continue;
        
        // Check each bit in the group
        for (int bit = 0; bit < 8; bit++) {
            uint8_t mask = 1 << bit;
            
            // Skip if this key is not pressed
            if (!(group_state & mask)) continue;
            
            // Create the combined key value
            CombinedKey current_key = MAKE_KEY(group, mask);
            
            // Store as the last key pressed
            last_key_pressed = current_key;
            
            // Trigger any-key callbacks if the key was just pressed
            for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
                if (callbacks[i].active && callbacks[i].is_any_key) {
                    // Check if the key was previously not pressed
                    bool was_any_pressed = false;
                    for (int g = 1; g <= 7; g++) {
                        if (kb_Data[g]) {
                            was_any_pressed = true;
                            break;
                        }
                    }
                    
                    // If this is a new key press, call the any-key callback
                    if (!was_any_pressed) {
                        callbacks[i].callback.any_press(current_key);
                    }
                }
            }
            
            // Since we found a pressed key, we can exit the loop
            break;
        }
    }

    // Process each active callback
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (!callbacks[i].active || callbacks[i].is_any_key) continue;

        CallbackEntry* cb = &callbacks[i];
        bool is_pressed = kb_is_key_pressed(cb->key);

        // Process based on callback type
        switch (cb->type) {
            case CB_PRESS:
                // Trigger on rising edge (key just pressed)
                if (is_pressed && !cb->was_pressed) {
                    cb->callback.press();
                }
                break;

            case CB_RELEASE:
                // Trigger on falling edge (key just released)
                if (!is_pressed && cb->was_pressed) {
                    cb->callback.release();
                }
                break;

            case CB_HOLD:
                // Track when key is first pressed
                if (is_pressed && !cb->was_pressed) {
                    cb->press_time = current_time;
                    cb->last_hold_time = 0;
                }

                // Check if we should trigger a hold event
                if (is_pressed && 
                    (current_time - cb->press_time >= (unsigned long)cb->hold_trigger_time)) {

                    // Initial trigger or repeat trigger
                    if (cb->last_hold_time == 0 || 
                        (cb->hold_repeat && 
                         (current_time - cb->last_hold_time >= (unsigned long)cb->hold_interval))) {

                        // Calculate how long the key has been held
                        int hold_duration = (int)(current_time - cb->press_time);

                        // Trigger the callback
                        cb->callback.hold(hold_duration);
                        cb->last_hold_time = current_time;
                    }
                }
                break;
        }

        // Update state for next iteration
        cb->was_pressed = is_pressed;
    }
}

/**
 * Waits for a key to be pressed and released.
 */
void kb_wait_any(void) {
    while (!kb_AnyKey()) {
        kb_Scan();
        delay(50);
    }
    debounce();
}

/**
 * Waits until no keys are pressed.
 */
void debounce(void) {
    last_key_pressed = 0;
    while (kb_AnyKey()) {
        kb_Scan();
        delay(50);
    }
}

/**
 * Registers a callback internally.
 * 
 * @param key The key to associate with the callback.
 * @param type The type of callback (press, release, hold).
 * @param callback_func The function pointer for the callback.
 * @param hold_time The time to wait before triggering a hold event (in ms).
 * @param repeat Whether the hold event should repeat.
 * @param repeat_interval The interval between hold events when repeating (in ms).
 * @return The ID of the registered callback, or -1 if registration failed.
 */
static KBCallbackID register_callback(CombinedKey key, CallbackType type, void* callback_func,
                                      int hold_time, bool repeat, int repeat_interval) {
    if (!initialized) kb_init();

    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) {
            slot = i;
            break;
        }
    }

    // If no slot available, return error
    if (slot < 0) return -1;

    // Set up the callback entry
    callbacks[slot].active = true;
    callbacks[slot].id = next_callback_id++;
    callbacks[slot].key = key;
    callbacks[slot].type = type;
    callbacks[slot].is_any_key = false;

    // Set the appropriate callback function
    switch (type) {
        case CB_PRESS:
            callbacks[slot].callback.press = (KBPressCallback)callback_func;
            break;
        case CB_RELEASE:
            callbacks[slot].callback.release = (KBReleaseCallback)callback_func;
            break;
        case CB_HOLD:
            callbacks[slot].callback.hold = (KBHoldCallback)callback_func;
            callbacks[slot].hold_trigger_time = hold_time;
            callbacks[slot].hold_repeat = repeat;
            callbacks[slot].hold_interval = repeat_interval;
            break;
    }

    callbacks[slot].was_pressed = false;

    return callbacks[slot].id;
}

/**
 * Gets the current time in milliseconds.
 * 
 * @return The current time in milliseconds, derived from the calculator's timer.
 */
static unsigned long get_millis(void) {
    // On the TI-84 CE, timer_1_Counter runs at 32768 Hz
    // Convert to milliseconds (approximately)
    return (unsigned long)(timer_1_Counter / 32.768);
}
