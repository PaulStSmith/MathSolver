#include <tice.h>
#include <keypadc.h>
#include <string.h>
#include "headers/kb_handler.h"

// Callback type enum
typedef enum {
    CB_PRESS,
    CB_RELEASE,
    CB_HOLD
} CallbackType;

// Callback entry structure
typedef struct {
    bool active;                // Whether this entry is active
    KBCallbackID id;            // Unique ID for this callback
    CombinedKey key;            // Which key this callback is for
    CallbackType type;          // Type of callback
    union {
        KBPressCallback press;    // Press callback function
        KBReleaseCallback release; // Release callback function
        KBHoldCallback hold;      // Hold callback function
    } callback;
    bool was_pressed;           // Previous state for edge detection
    unsigned long press_time;   // Time when key was first pressed (for hold detection)
    int hold_trigger_time;      // Time to wait before triggering hold event (ms)
    bool hold_repeat;           // Whether hold event should repeat
    int hold_interval;          // Interval between hold events when repeating
    unsigned long last_hold_time; // Time of last hold event
} CallbackEntry;

// Static state
static CallbackEntry callbacks[KB_MAX_CALLBACKS];
static int next_callback_id = 1; // Start at 1 so 0 can be invalid
static bool initialized = false;

// Private function prototypes
static KBCallbackID register_callback(CombinedKey key, CallbackType type, void* callback_func,
                                     int hold_time, bool repeat, int repeat_interval);
static unsigned long get_millis(void);

/**
 * Initialize the keyboard handler
 */
void kb_init(void) {
    if (initialized) return;
    
    // Clear all callback entries
    memset(callbacks, 0, sizeof(callbacks));
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        callbacks[i].active = false;
    }
    
    next_callback_id = 1;
    initialized = true;
}

/**
 * Register a press callback
 */
KBCallbackID kb_register_press(CombinedKey key, KBPressCallback callback) {
    return register_callback(key, CB_PRESS, (void*)callback, 0, false, 0);
}

/**
 * Register a release callback
 */
KBCallbackID kb_register_release(CombinedKey key, KBReleaseCallback callback) {
    return register_callback(key, CB_RELEASE, (void*)callback, 0, false, 0);
}

/**
 * Register a hold callback
 */
KBCallbackID kb_register_hold(CombinedKey key, KBHoldCallback callback, 
                             int trigger_time_ms, bool repeat, int repeat_interval_ms) {
    return register_callback(key, CB_HOLD, (void*)callback, 
                            trigger_time_ms, repeat, repeat_interval_ms);
}

/**
 * Unregister a callback by ID
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
 * Unregister all callbacks for a key
 */
int kb_unregister(CombinedKey key) {
    if (!initialized) kb_init();
    
    int count = 0;
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (callbacks[i].active && callbacks[i].key == key) {
            callbacks[i].active = false;
            count++;
        }
    }
    
    return count;
}

/**
 * Clear all callbacks
 */
void kb_clear(void) {
    if (!initialized) return;
    
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        callbacks[i].active = false;
    }
}

/**
 * Process keyboard events
 */
void kb_process(void) {
    if (!initialized) kb_init();
    
    // Get current time for hold timing
    unsigned long current_time = get_millis();
    
    // Scan keyboard once
    kb_Scan();
    
    // Process each active callback
    for (int i = 0; i < KB_MAX_CALLBACKS; i++) {
        if (!callbacks[i].active) continue;
        
        CallbackEntry* cb = &callbacks[i];
        bool is_pressed = is_key_pressed(cb->key);
        
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
 * Register a callback (internal function)
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
 * Get current time in milliseconds
 * Uses the calculator's timer functionality
 */
static unsigned long get_millis(void) {
    // On the TI-84 CE, timer_1_Counter runs at 32768 Hz
    // Convert to milliseconds (approximately)
    return (unsigned long)(timer_1_Counter / 32.768);
}
