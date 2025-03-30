#include <tice.h>
#include <string.h>
#include <stdio.h>
#include "headers/keyboard.h"
#include "headers/key_translator_private.h"

#include "headers/log.h"

/** Maximum number of character callbacks that can be registered. */
#define MAX_CHAR_CALLBACKS 16

/** 
 * Internal structure for callback registration.
 * Stores information about a registered callback, including its type, ID, and function pointer.
 */
typedef struct {
    bool active;                     /**< Whether this entry is active. */
    int id;                          /**< Unique ID for this callback. */
    union {
        CharDownCallback down;       /**< Function pointer for down callback. */
        CharPressCallback press;     /**< Function pointer for press callback. */
        CharUpCallback up;           /**< Function pointer for up callback. */
    } callback;                      /**< Union to store the callback function pointer. */
    int type;                        /**< Type of callback (0=down, 1=press, 2=up). */
    int repeat_delay;                /**< Time before repeating (for press). */
    int repeat_interval;             /**< Interval between repeats (for press). */
} CharCallbackEntry;

/** Callback types for character events. */
enum {
    CHAR_CB_DOWN = 0,  /**< Callback for key down events. */
    CHAR_CB_PRESS = 1, /**< Callback for key press events. */
    CHAR_CB_UP = 2     /**< Callback for key up events. */
};

// Static state variables
/** Array of callback entries. */
static CharCallbackEntry char_callbacks[MAX_CHAR_CALLBACKS];
/** Next available callback ID. */
static int char_next_callback_id = 1;
/** Whether the translator is initialized. */
static bool char_initialized = false;
/** Current keyboard mode. */
static KeyboardMode current_mode = KB_MODE_NORMAL;
/** Last translated key value. */
static int last_key_value = CHAR_NULL;
/** Last physical key. */
static Key last_physical_key = KEY_NONE;
/** IDs of registered key callbacks. */
static int key_callback_ids[3] = {-1, -1, -1};

/**
 * Initialize the key translator subsystem.
 * Sets up the necessary state and registers callbacks with the keyboard layer.
 */
void char_init(void) {
    if (char_initialized) {
        log_message("char_init: Already initialized.");
        return;
    }
    
    log_message("char_init: Initializing key translator subsystem.");
    
    // Ensure keyboard layer is initialized
    key_init();
    
    // Clear all callback entries
    memset(char_callbacks, 0, sizeof(char_callbacks));
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        char_callbacks[i].active = false;
    }
    
    // Reset state
    char_next_callback_id = 1;
    current_mode = KB_MODE_NORMAL;
    last_key_value = CHAR_NULL;
    last_physical_key = KEY_NONE;
    
    // Register callbacks with keyboard layer
    key_callback_ids[0] = key_register_down(on_key_down);
    key_callback_ids[1] = key_register_press(on_key_press, 500, 100);
    key_callback_ids[2] = key_register_up(on_key_up);
    
    char_initialized = true;
    log_message("char_init: Key translator subsystem initialized.");
}

/**
 * Clean up and release resources.
 * Unregisters callbacks and resets the state.
 */
void char_deinit(void) {
    if (!char_initialized) {
        log_message("char_deinit: Subsystem not initialized.");
        return;
    }
    
    log_message("char_deinit: Cleaning up key translator subsystem.");
    
    // Unregister keyboard callbacks
    for (int i = 0; i < 3; i++) {
        if (key_callback_ids[i] != -1) {
            key_unregister(key_callback_ids[i]);
            key_callback_ids[i] = -1;
        }
    }
    
    char_initialized = false;
    log_message("char_deinit: Key translator subsystem cleaned up.");
}

/**
 * Register a callback for character down events.
 * 
 * @param callback The function to call when a key is pressed down.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int char_register_down(CharDownCallback callback) {
    log_message("char_register_down: Registering down callback.");
    if (!char_initialized) char_init();
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (!char_callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("char_register_down: No available slot for callback.");
        return -1;
    }
    
    // Set up the callback entry
    char_callbacks[slot].active = true;
    char_callbacks[slot].id = char_next_callback_id++;
    char_callbacks[slot].callback.down = callback;
    char_callbacks[slot].type = CHAR_CB_DOWN;
    
    log_message("char_register_down: Down callback registered successfully.");
    return char_callbacks[slot].id;
}

/**
 * Register a callback for character press events.
 * 
 * @param callback The function to call when a key is pressed.
 * @param repeat_delay_ms Time in milliseconds before repeating the press event.
 * @param repeat_interval_ms Interval in milliseconds between repeated press events.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int char_register_press(CharPressCallback callback, int repeat_delay_ms, int repeat_interval_ms) {
    log_message("char_register_press: Registering press callback.");
    if (!char_initialized) char_init();
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (!char_callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("char_register_press: No available slot for callback.");
        return -1;
    }
    
    // Set up the callback entry
    char_callbacks[slot].active = true;
    char_callbacks[slot].id = char_next_callback_id++;
    char_callbacks[slot].callback.press = callback;
    char_callbacks[slot].type = CHAR_CB_PRESS;
    char_callbacks[slot].repeat_delay = repeat_delay_ms;
    char_callbacks[slot].repeat_interval = repeat_interval_ms;
    
    log_message("char_register_press: Press callback registered successfully.");
    return char_callbacks[slot].id;
}

/**
 * Register a callback for character up events.
 * 
 * @param callback The function to call when a key is released.
 * @return The ID of the registered callback, or -1 if registration failed.
 */
int char_register_up(CharUpCallback callback) {
    log_message("char_register_up: Registering up callback.");
    if (!char_initialized) char_init();
    
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (!char_callbacks[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no slot available, return error
    if (slot < 0) {
        log_message("char_register_up: No available slot for callback.");
        return -1;
    }
    
    // Set up the callback entry
    char_callbacks[slot].active = true;
    char_callbacks[slot].id = char_next_callback_id++;
    char_callbacks[slot].callback.up = callback;
    char_callbacks[slot].type = CHAR_CB_UP;
    
    log_message("char_register_up: Up callback registered successfully.");
    return char_callbacks[slot].id;
}

/**
 * Unregister a callback by its ID.
 * 
 * @param callback_id The ID of the callback to unregister.
 * @return True if the callback was successfully unregistered, false otherwise.
 */
bool char_unregister(int callback_id) {
    log_message("char_unregister: Unregistering callback with ID %d.", callback_id);
    if (!char_initialized) return false;
    
    bool success = false;
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (char_callbacks[i].active && char_callbacks[i].id == callback_id) {
            char_callbacks[i].active = false;
            success = true;
            break;
        }
    }
    
    if (success) {
        log_message("char_unregister: Callback unregistered successfully.");
    } else {
        log_message("char_unregister: Callback ID %d not found.", callback_id);
    }
    return success;
}

/**
 * Clear all registered callbacks.
 * Deactivates all callback entries.
 */
void char_clear_callbacks(void) {
    log_message("char_clear_callbacks: Clearing all registered callbacks.");
    if (!char_initialized) return;
    
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        char_callbacks[i].active = false;
    }
    log_message("char_clear_callbacks: All callbacks cleared.");
}

/**
 * Process keyboard events once and trigger character callbacks.
 * Calls the keyboard layer's scan function to process key events.
 */
void char_scan(void) {
    log_message("char_scan: Scanning for keyboard events.");
    if (!char_initialized) char_init();
    
    // Let the keyboard layer process key events
    key_scan();
}

/**
 * Wait for any character input.
 * Blocks until a key is pressed and returns the translated character value.
 * 
 * @return The translated character value of the pressed key.
 */
int char_wait_any(void) {
    log_message("char_wait_any: Waiting for any character input.");
    if (!char_initialized) char_init();
    
    last_key_value = CHAR_NULL;
    
    // Wait for a translated key value
    while (last_key_value == CHAR_NULL) {
        char_scan();
        delay(10);
    }
    
    int result = last_key_value;
    last_key_value = CHAR_NULL;
    
    log_message("char_wait_any: Received character input: %d.", result);
    return result;
}

/**
 * Get the current keyboard mode.
 * 
 * @return The current keyboard mode.
 */
KeyboardMode char_get_mode(void) {
    log_message("char_get_mode: Current mode is %d.", current_mode);
    return current_mode;
}

/**
 * Set the keyboard mode.
 * 
 * @param mode The new keyboard mode to set.
 */
void char_set_mode(KeyboardMode mode) {
    log_message("char_set_mode: Setting mode to %d.", mode);
    current_mode = mode;
}

/**
 * Process a special mode key (2nd, Alpha) and update the keyboard mode.
 * 
 * @param key The key to process.
 * @return True if the key was handled as a mode key, false otherwise.
 */
bool char_process_mode_key(Key key) {
    log_message("char_process_mode_key: Processing mode key %d.", key);
    bool handled = false;
    
    if (key == KEY_2ND) {
        // 2nd key pressed - toggle 2nd mode
        current_mode ^= KB_MODE_2ND;
        handled = true;
    } 
    else if (key == KEY_ALPHA) {
        // Alpha key pressed
        if (current_mode & ~KB_MODE_ALPHA) {
            current_mode |= KB_MODE_ALPHA;                      // Set alpha mode
        } else if (current_mode & KB_MODE_ALPHA) {
            // If we are in alpha mode, change to alpha lower
            current_mode |= KB_MODE_ALPHA_LOWER;                // Set alpha lower mode
        } else if (current_mode & KB_MODE_LOWER) {
            // If we are in lower mode, return to normal mode
            current_mode = current_mode & ~KB_MODE_ALPHA_LOWER; // Remove alpha lower mode
            current_mode = current_mode & ~KB_MODE_LOCK;        // Remove locked mode
        }
        
        if (current_mode & KB_MODE_2ND) {
            current_mode |= KB_MODE_LOCK;                       // Set locked mode
        } 
        current_mode = current_mode & ~KB_MODE_2ND;             // Remove 2nd mode
        handled = true;
    }
    
    if (handled) {
        log_message("char_process_mode_key: Key %d handled as mode key.", key);
    } else {
        log_message("char_process_mode_key: Key %d not handled as mode key.", key);
    }
    return handled;
}

/**
 * Translate a physical key to a character value based on the current keyboard mode.
 * 
 * @param key The physical key to translate.
 * @return The translated character value, or CHAR_NULL if no mapping exists.
 */
int char_translate_key(Key key) {
    log_message("char_translate_key: Translating key %d.", key);
    int group = KEY_GROUP(key);
    int mask = KEY_MASK(key);
    bool is_2nd = (current_mode & KB_MODE_2ND) != 0;
    bool is_alpha = (current_mode & KB_MODE_ALPHA) != 0;
    bool is_lower = (current_mode & KB_MODE_LOWER) != 0;
    
    // Special mode keys - not translated
    if (key == KEY_2ND || key == KEY_ALPHA) {
        return (key == KEY_2ND) ? CHAR_2ND : CHAR_ALPHA;
    }
    
    // Handle normal mode (no alpha, no 2nd)
    if (!is_2nd && !is_alpha) {
        // Group 1: Graph, Trace, Zoom, Window, Y=, 2nd, Mode, Del
        if (group == 1) {
            if (mask == kb_Graph)    return FUNC_GRAPH;
            if (mask == kb_Trace)    return FUNC_TRACE;
            if (mask == kb_Zoom)     return FUNC_ZOOM;
            if (mask == kb_Window)   return FUNC_WINDOW;
            if (mask == kb_Yequ)     return FUNC_Y_EQUALS;
            if (mask == kb_2nd)      return CHAR_2ND;     // Should be handled by char_process_mode_key
            if (mask == kb_Mode)     return CHAR_MODE;
            if (mask == kb_Del)      return CHAR_DEL;
        }
        // Group 2: Sto, Ln, Log, x², 1/x, Math, Alpha
        else if (group == 2) {
            if (mask == kb_Sto)      return FUNC_STO;
            if (mask == kb_Ln)       return FUNC_LN;
            if (mask == kb_Log)      return FUNC_LOG;
            if (mask == kb_Square)   return FUNC_SQUARE;
            if (mask == kb_Recip)    return FUNC_RECIP;
            if (mask == kb_Math)     return FUNC_MATH;
            if (mask == kb_Alpha)    return CHAR_ALPHA;   // Should be handled by char_process_mode_key
        }
        // Group 3: 0, 1, 4, 7, ,, sin, apps, x
        else if (group == 3) {
            if (mask == kb_0)        return '0';
            if (mask == kb_1)        return '1';
            if (mask == kb_4)        return '4';
            if (mask == kb_7)        return '7';
            if (mask == kb_Comma)    return ',';
            if (mask == kb_Sin)      return FUNC_SIN;
            if (mask == kb_Apps)     return FUNC_APPS;
            if (mask == kb_GraphVar) return FUNC_X_VAR;
        }
        // Group 4: ., 2, 5, 8, (, cos, prgm, stat
        else if (group == 4) {
            if (mask == kb_DecPnt)  return '.';
            if (mask == kb_2)       return '2';
            if (mask == kb_5)       return '5';
            if (mask == kb_8)       return '8';
            if (mask == kb_LParen)  return '(';
            if (mask == kb_Cos)     return FUNC_COS;
            if (mask == kb_Prgm)    return FUNC_PRGM;
            if (mask == kb_Stat)    return FUNC_STAT;
        }
        // Group 5: (-), 3, 6, 9, ), tan, vars
        else if (group == 5) {
            if (mask == kb_Chs)      return '-';
            if (mask == kb_3)        return '3';
            if (mask == kb_6)        return '6';
            if (mask == kb_9)        return '9';
            if (mask == kb_RParen)   return ')';
            if (mask == kb_Tan)      return FUNC_TAN;
            if (mask == kb_Vars)     return FUNC_VARS;
        }
        // Group 6: Enter, +, -, *, /, ^, clear
        else if (group == 6) {
            if (mask == kb_Enter)    return CHAR_ENTER;
            if (mask == kb_Add)      return '+';
            if (mask == kb_Sub)      return '-';
            if (mask == kb_Mul)      return '*';
            if (mask == kb_Div)      return '/';
            if (mask == kb_Power)    return '^';
            if (mask == kb_Clear)    return CHAR_CLEAR;
        }
        // Group 7: down, left, right, up
        else if (group == 7) {
            if (mask == kb_Down)     return CHAR_DOWN;
            if (mask == kb_Left)     return CHAR_LEFT;
            if (mask == kb_Right)    return CHAR_RIGHT;
            if (mask == kb_Up)       return CHAR_UP;
        }
    }
    // Handle alpha modes (both regular and lowercase)
    else if (is_alpha && !is_2nd) {
        // Per the Keyboard Modes.md file
        if (group == 2) {
            if (mask == kb_Math)     return is_lower ? 'a' : 'A';
            if (mask == kb_Recip)    return is_lower ? 'd' : 'D';
            if (mask == kb_Square)   return is_lower ? 'i' : 'I';
            if (mask == kb_Log)      return is_lower ? 'n' : 'N';
            if (mask == kb_Ln)       return is_lower ? 's' : 'S';
            if (mask == kb_Sto)      return is_lower ? 'x' : 'X';
        }
        else if (group == 3) {
            if (mask == kb_Apps)     return is_lower ? 'b' : 'B';
            if (mask == kb_Sin)      return is_lower ? 'e' : 'E';
            if (mask == kb_7)        return is_lower ? 'o' : 'O';
            if (mask == kb_4)        return is_lower ? 't' : 'T';
            if (mask == kb_1)        return is_lower ? 'y' : 'Y';
            if (mask == kb_0)        return ' ';  // Space
            if (mask == kb_Comma)    return is_lower ? 'j' : 'J';
        }
        else if (group == 4) {
            if (mask == kb_Prgm)     return is_lower ? 'c' : 'C';
            if (mask == kb_Cos)      return is_lower ? 'f' : 'F';
            if (mask == kb_8)        return is_lower ? 'p' : 'P';
            if (mask == kb_5)        return is_lower ? 'u' : 'U';
            if (mask == kb_2)        return is_lower ? 'z' : 'Z';
            if (mask == kb_DecPnt)   return ':';
            if (mask == kb_LParen)   return is_lower ? 'k' : 'K';
        }
        else if (group == 5) {
            if (mask == kb_Tan)      return is_lower ? 'g' : 'G';
            if (mask == kb_6)        return is_lower ? 'v' : 'V';
            if (mask == kb_3)        return '[';  // No lowercase for special chars
            if (mask == kb_Chs)      return '?';
            if (mask == kb_9)        return is_lower ? 'q' : 'Q';
            if (mask == kb_RParen)   return is_lower ? 'l' : 'L';
        }
        else if (group == 6) {
            if (mask == kb_Power)    return is_lower ? 'h' : 'H';
            if (mask == kb_Div)      return is_lower ? 'm' : 'M';
            if (mask == kb_Mul)      return is_lower ? 'r' : 'R';
            if (mask == kb_Sub)      return is_lower ? 'w' : 'W';
            if (mask == kb_Add)      return '"';
            // Keep other keys the same
            if (mask == kb_Enter)    return CHAR_ENTER;
            if (mask == kb_Clear)    return CHAR_CLEAR;
        }
        // Arrow keys (Group 7) - keep the same in all modes
        else if (group == 7) {
            if (mask == kb_Down)     return CHAR_DOWN;
            if (mask == kb_Left)     return CHAR_LEFT;
            if (mask == kb_Right)    return CHAR_RIGHT;
            if (mask == kb_Up)       return CHAR_UP;
        }
    }
    // Handle 2nd mode
    else if (is_2nd) {
        // Per the Keyboard Modes.md file
        if (group == 2) {
            if (mask == kb_Recip)    return FUNC_X_INV;     // ^-1
            if (mask == kb_Square)   return FUNC_ROOT;      // sqrt(
            if (mask == kb_Log)      return FUNC_10_X;      // 10^
            if (mask == kb_Ln)       return FUNC_EXP;       // e^x
            if (mask == kb_Sto)      return FUNC_RECALL;
            if (mask == kb_Math)     return FUNC_TEST;
        }
        else if (group == 3) {
            if (mask == kb_Sin)      return FUNC_SIN_INV;   // asin(
            if (mask == kb_7)        return 'u';
            if (mask == kb_Apps)     return FUNC_MATRIX;
            if (mask == kb_GraphVar) return FUNC_DRAW;
            if (mask == kb_4)        return FUNC_ANGLE;
        }
        else if (group == 4) {
            if (mask == kb_Cos)      return FUNC_COS_INV;   // acos(
            if (mask == kb_8)        return 'v';
            if (mask == kb_LParen)   return '{';
            if (mask == kb_Prgm)     return FUNC_LIST;
            if (mask == kb_Stat)     return FUNC_PROBABILITY;
            if (mask == kb_5)        return FUNC_MEM;
        }
        else if (group == 5) {
            if (mask == kb_Tan)      return FUNC_TAN_INV;   // atan(
            if (mask == kb_9)        return 'w';
            if (mask == kb_RParen)   return '}';
            if (mask == kb_Chs)      return FUNC_ENTRY;
            if (mask == kb_Vars)     return FUNC_STRING;
            if (mask == kb_3)        return FUNC_SOLVE;
            if (mask == kb_6)        return FUNC_PARAMETRIC;
        }
        else if (group == 6) {
            if (mask == kb_Power)    return FUNC_PI;        // π
            if (mask == kb_Div)      return 'e';            // Constant e
            if (mask == kb_Mul)      return '[';
            if (mask == kb_Sub)      return ']';
            if (mask == kb_Enter)    return CHAR_ENTER;
            if (mask == kb_Add)      return FUNC_MEM_ADD;
            if (mask == kb_Clear)    return FUNC_RESET;
        }
        else if (group == 7) {
            if (mask == kb_Up)       return CHAR_PGUP;
            if (mask == kb_Down)     return CHAR_PGDN;
            if (mask == kb_Left)     return CHAR_HOME;
            if (mask == kb_Right)    return CHAR_END;
        }
    }
    
    // Default: return CHAR_NULL if no mapping found
    int result = CHAR_NULL;
    log_message("char_translate_key: Translated key %d to character value %d.", key, result);
    return result;
}

/**
 * Convert a character value to a string representation for display/debug.
 * 
 * @param value The character value to convert.
 * @param buffer The buffer to store the string representation.
 */
void char_value_to_string(int value, char* buffer) {
    log_message("char_value_to_string: Converting value %d to string.", value);
    if (!buffer) return;
    
    // Handle special control characters
    if (value < 32) {
        switch (value) {
            case CHAR_NULL:    strcpy(buffer, "NULL");  break;
            case CHAR_ENTER:   strcpy(buffer, "ENTER"); break;
            case CHAR_CLEAR:   strcpy(buffer, "CLEAR"); break;
            case CHAR_DEL:     strcpy(buffer, "DEL");   break;
            case CHAR_UP:      strcpy(buffer, "UP");    break;
            case CHAR_DOWN:    strcpy(buffer, "DOWN");  break;
            case CHAR_LEFT:    strcpy(buffer, "LEFT");  break;
            case CHAR_RIGHT:   strcpy(buffer, "RIGHT"); break;
            case CHAR_2ND:     strcpy(buffer, "2ND");   break;
            case CHAR_ALPHA:   strcpy(buffer, "ALPHA"); break;
            case CHAR_MODE:    strcpy(buffer, "MODE");  break;
            case CHAR_HOME:    strcpy(buffer, "HOME");  break;
            case CHAR_END:     strcpy(buffer, "END");   break;
            case CHAR_INS:     strcpy(buffer, "INS");   break;
            case CHAR_PGUP:    strcpy(buffer, "PGUP");  break;
            case CHAR_PGDN:    strcpy(buffer, "PGDN");  break;
            default:           sprintf(buffer, "CTRL-%d", value);
        }
    }
    // Handle normal ASCII characters
    else if (value >= 32 && value < 128) {
        buffer[0] = (char)value;
        buffer[1] = '\0';
    }
    // Handle function keys and special symbols
    else if (value >= 128) {
        switch (value) {
            case FUNC_Y_EQUALS:     strcpy(buffer, "Y=");       break;
            case FUNC_WINDOW:       strcpy(buffer, "WINDOW");   break;
            case FUNC_ZOOM:         strcpy(buffer, "ZOOM");     break;
            case FUNC_TRACE:        strcpy(buffer, "TRACE");    break;
            case FUNC_GRAPH:        strcpy(buffer, "GRAPH");    break;
            case FUNC_SIN:          strcpy(buffer, "SIN");      break;
            case FUNC_COS:          strcpy(buffer, "COS");      break;
            case FUNC_TAN:          strcpy(buffer, "TAN");      break;
            case FUNC_LOG:          strcpy(buffer, "LOG");      break;
            case FUNC_LN:           strcpy(buffer, "LN");       break;
            case FUNC_STO:          strcpy(buffer, "STO→");     break;
            case FUNC_SQUARE:       strcpy(buffer, "X²");       break;
            case FUNC_RECIP:        strcpy(buffer, "1/X");      break;
            case FUNC_MATH:         strcpy(buffer, "MATH");     break;
            case FUNC_APPS:         strcpy(buffer, "APPS");     break;
            case FUNC_PRGM:         strcpy(buffer, "PRGM");     break;
            case FUNC_VARS:         strcpy(buffer, "VARS");     break;
            case FUNC_X_VAR:        strcpy(buffer, "X");        break;
            case FUNC_SIN_INV:      strcpy(buffer, "SIN⁻¹");    break;
            case FUNC_COS_INV:      strcpy(buffer, "COS⁻¹");    break;
            case FUNC_TAN_INV:      strcpy(buffer, "TAN⁻¹");    break;
            case FUNC_PI:           strcpy(buffer, "π");        break;
            case FUNC_ROOT:         strcpy(buffer, "√");        break;
            default:                sprintf(buffer, "FUNC-%d", value - 128);
        }
    }
    else {
        // Handle any other values
        sprintf(buffer, "VAL-%d", value);
    }
    log_message("char_value_to_string: Converted value %d to string '%s'.", value, buffer);
}

/**
 * Handle key down events from the keyboard layer.
 * 
 * @param key The key that was pressed down.
 */
static void on_key_down(Key key) {
    log_message("on_key_down: Key down event for key %d.", key);
    // Process mode keys first
    if (char_process_mode_key(key)) {
        return;
    }
    
    // Translate the key to a character value
    int value = char_translate_key(key);
    last_key_value = value;
    last_physical_key = key;
    
    // Trigger char_down callbacks
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (char_callbacks[i].active && char_callbacks[i].type == CHAR_CB_DOWN) {
            char_callbacks[i].callback.down(value);
        }
    }
    
    // Auto-reset 2nd mode after key press (unless in Alpha Lock or Alpha Lower Lock)
    if ((current_mode & KB_MODE_2ND) && !(current_mode & KB_MODE_LOCK)) {
        current_mode &= ~KB_MODE_2ND;
    }
    
    // Auto-reset Alpha mode after key press (unless in Alpha Lock or Alpha Lower Lock)
    if ((current_mode & KB_MODE_ALPHA) && !(current_mode & KB_MODE_LOCK)) {
        current_mode &= ~(KB_MODE_ALPHA | KB_MODE_LOWER);
    }
    log_message("on_key_down: Key down event processed for key %d.", key);
}

/**
 * Handle key press events from the keyboard layer.
 * 
 * @param key The key that was pressed.
 */
static void on_key_press(Key key) {
    log_message("on_key_press: Key press event for key %d.", key);
    // Skip if this is a mode key or if it's different from our last key
    if (key == KEY_2ND || key == KEY_ALPHA || key != last_physical_key) {
        return;
    }
    
    // Trigger char_press callbacks
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (char_callbacks[i].active && char_callbacks[i].type == CHAR_CB_PRESS) {
            char_callbacks[i].callback.press(last_key_value);
        }
    }
    log_message("on_key_press: Key press event processed for key %d.", key);
}

/**
 * Handle key up events from the keyboard layer.
 * 
 * @param key The key that was released.
 */
static void on_key_up(Key key) {
    log_message("on_key_up: Key up event for key %d.", key);
    // Skip if this is a mode key or if it's different from our last key
    if (key == KEY_2ND || key == KEY_ALPHA || key != last_physical_key) {
        return;
    }
    
    // Trigger char_up callbacks
    for (int i = 0; i < MAX_CHAR_CALLBACKS; i++) {
        if (char_callbacks[i].active && char_callbacks[i].type == CHAR_CB_UP) {
            char_callbacks[i].callback.up(last_key_value);
        }
    }
    
    // Reset last key values
    last_key_value = CHAR_NULL;
    last_physical_key = KEY_NONE;
    log_message("on_key_up: Key up event processed for key %d.", key);
}
