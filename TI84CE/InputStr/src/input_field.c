#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "headers/gui.h"
#include "headers/kb_handler.h"
#include "headers/kb_mapping.h"
#include "headers/input_field.h"

#define LOG_TAG "input_field"
#include "headers/log.h"

// Default initial buffer size
#define INITIAL_BUFFER_SIZE 64

// Forward declarations of internal functions
static bool ensure_buffer_size(InputField* field, int needed_size);
static void ensure_cursor_visible(InputField* field);
static void process_character_input(InputField* field, char c);
static void process_function_key(InputField* field, int func_code);
static void process_mode_key(InputField* field, CombinedKey key);
static void draw_mode_indicator_default(KeyboardMode mode, int x, int y);

/**
 * Initialize an input field.
 * 
 * @param field Pointer to InputField structure
 * @param x X position
 * @param y Y position
 * @param width Width of text field in pixels
 * @param has_border Whether to draw a border
 */
void input_field_init(InputField* field, int x, int y, int width, bool has_border) {
    log_debug("input_field_init() called");
    
    memset(field, 0, sizeof(InputField));
    field->x = x;
    field->y = y;
    field->width = width;
    field->has_border = has_border;
    
    // Calculate maximum visible characters
    field->max_visible_chars = (width - 2*PADDING_X) / GUI_CHAR_WIDTH;
    
    // Allocate initial buffer
    field->buffer_size = INITIAL_BUFFER_SIZE;
    field->text = (char*)malloc(field->buffer_size);
    if (field->text) {
        field->text[0] = '\0';
    } else {
        log_error("Failed to allocate text buffer");
    }
    
    // Initialize other values
    field->text_length = 0;
    field->cursor_position = 0;
    field->scroll_offset = 0;
    field->is_active = false;
    field->kbd_mode = KB_MODE_NORMAL;
    field->next_field = NULL;
    
    // Set default mode indicator callback
    field->mode_indicator_callback = draw_mode_indicator_default;
}

/**
 * Free resources used by an input field.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_free(InputField* field) {
    log_debug("input_field_free() called");
    
    if (field->text) {
        free(field->text);
        field->text = NULL;
    }
    field->buffer_size = 0;
    field->text_length = 0;
}

/**
 * Ensure the buffer is large enough for the given size.
 * 
 * @param field Pointer to InputField structure
 * @param needed_size Size needed including null terminator
 * @return true if buffer is large enough or was successfully resized
 */
static bool ensure_buffer_size(InputField* field, int needed_size) {
    if (!field->text) return false;
    
    if (needed_size <= field->buffer_size) {
        return true; // Already large enough
    }
    
    // Calculate new size (double until sufficient)
    int new_size = field->buffer_size;
    while (new_size < needed_size) {
        new_size *= 2;
    }
    
    // Reallocate
    char* new_buffer = (char*)realloc(field->text, new_size);
    if (!new_buffer) {
        log_error("Failed to resize text buffer");
        return false;
    }
    
    field->text = new_buffer;
    field->buffer_size = new_size;
    return true;
}

/**
 * Clear all text from the field.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_clear(InputField* field) {
    log_debug("input_field_clear() called");
    
    if (field->text) {
        field->text[0] = '\0';
        field->text_length = 0;
        field->cursor_position = 0;
        field->scroll_offset = 0;
    }
}

/**
 * Set the text content of the field.
 * 
 * @param field Pointer to InputField structure
 * @param text Text to set
 */
void input_field_set_text(InputField* field, const char* text) {
    log_debug("input_field_set_text() called");
    
    int length = strlen(text);
    if (ensure_buffer_size(field, length + 1)) {
        strcpy(field->text, text);
        field->text_length = length;
        
        // Reset cursor and scroll
        field->cursor_position = length;
        ensure_cursor_visible(field);
    }
}

/**
 * Get the current text content of the field.
 * 
 * @param field Pointer to InputField structure
 * @return The current text content
 */
const char* input_field_get_text(InputField* field) {
    return field->text;
}

/**
 * Append text to the current content.
 * 
 * @param field Pointer to InputField structure
 * @param text Text to append
 */
void input_field_append(InputField* field, const char* text) {
    log_debug("input_field_append() called");
    
    int append_length = strlen(text);
    int new_length = field->text_length + append_length;
    
    if (ensure_buffer_size(field, new_length + 1)) {
        strcat(field->text, text);
        field->text_length = new_length;
        
        // Move cursor to end
        field->cursor_position = new_length;
        ensure_cursor_visible(field);
    }
}

/**
 * Insert a character at the current cursor position.
 * 
 * @param field Pointer to InputField structure
 * @param c Character to insert
 */
void input_field_insert_char(InputField* field, char c) {
    log_debug("input_field_insert_char() called");
    
    if (ensure_buffer_size(field, field->text_length + 2)) {
        // Shift characters after cursor
        for (int i = field->text_length; i >= field->cursor_position; i--) {
            field->text[i + 1] = field->text[i];
        }
        
        // Insert the new character
        field->text[field->cursor_position] = c;
        field->text_length++;
        field->cursor_position++;
        
        ensure_cursor_visible(field);
    }
}

/**
 * Delete the character before the cursor (backspace).
 * 
 * @param field Pointer to InputField structure
 */
void input_field_backspace(InputField* field) {
    log_debug("input_field_backspace() called");
    
    if (field->cursor_position > 0) {
        // Shift characters at and after cursor
        for (int i = field->cursor_position - 1; i < field->text_length; i++) {
            field->text[i] = field->text[i + 1];
        }
        
        field->text_length--;
        field->cursor_position--;
        
        ensure_cursor_visible(field);
    }
}

/**
 * Delete the character at the cursor position.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_delete(InputField* field) {
    log_debug("input_field_delete() called");
    
    // Only proceed if we're not at the end of the text
    if (field->cursor_position < field->text_length) {
        // Shift characters after cursor
        for (int i = field->cursor_position; i < field->text_length; i++) {
            field->text[i] = field->text[i + 1];
        }
        
        field->text_length--;
        
        // Cursor position stays the same
        ensure_cursor_visible(field);
    }
}

/**
 * Move cursor left one character.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_cursor_left(InputField* field) {
    if (field->cursor_position > 0) {
        field->cursor_position--;
        ensure_cursor_visible(field);
    }
}

/**
 * Move cursor right one character.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_cursor_right(InputField* field) {
    if (field->cursor_position < field->text_length) {
        field->cursor_position++;
        ensure_cursor_visible(field);
    }
}

/**
 * Move cursor to the start of the text.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_cursor_to_start(InputField* field) {
    field->cursor_position = 0;
    field->scroll_offset = 0;
}

/**
 * Move cursor to the end of the text.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_cursor_to_end(InputField* field) {
    field->cursor_position = field->text_length;
    ensure_cursor_visible(field);
}

/**
 * Ensure the cursor is visible in the current scroll view.
 * 
 * @param field Pointer to InputField structure
 */
static void ensure_cursor_visible(InputField* field) {
    // If cursor is to the left of visible area
    if (field->cursor_position < field->scroll_offset) {
        field->scroll_offset = field->cursor_position;
    }
    
    // If cursor is to the right of visible area
    else if (field->cursor_position >= field->scroll_offset + field->max_visible_chars) {
        field->scroll_offset = field->cursor_position - field->max_visible_chars + 1;
    }
}

/**
 * Process a character input in the current mode.
 * 
 * @param field Pointer to InputField structure
 * @param c Character to process
 */
static void process_character_input(InputField* field, char c) {
    if (c >= 32 && c <= 126) {  // Printable ASCII
        input_field_insert_char(field, c);
    }
}

/**
 * Process a function key in the current mode.
 * 
 * @param field Pointer to InputField structure
 * @param func_code Function code to process
 */
static void process_function_key(InputField* field, int func_code) {
    // Handle function keys based on their meaning
    switch (func_code) {
        case FUNC_SIN:
            input_field_append(field, "sin(");
            break;
        case FUNC_COS:
            input_field_append(field, "cos(");
            break;
        case FUNC_TAN:
            input_field_append(field, "tan(");
            break;
        case FUNC_LN:
            input_field_append(field, "ln(");
            break;
        case FUNC_LOG:
            input_field_append(field, "log(");
            break;
        case FUNC_SQUARE:
            input_field_append(field, "^2");
            break;
        case FUNC_RECIP:
            input_field_append(field, "^(-1)");
            break;
        case FUNC_X_VAR:
            input_field_insert_char(field, 'x');
            break;
        case FUNC_PI:
            input_field_append(field, "\xC4");  // π
            break;
        case FUNC_EXP:
            input_field_append(field, "\xDB^("); // e^x
            break;
        case FUNC_COS_INV:
            input_field_append(field, "acos(");
            break;
        case FUNC_SIN_INV:
            input_field_append(field, "asin(");
            break;
        case FUNC_TAN_INV:
            input_field_append(field, "atan(");
            break;
        case FUNC_10_X:
            input_field_append(field, "10^(");
            break;
        case FUNC_ROOT:
            input_field_append(field, "sqrt(");
            break;
        // Add more function key handlers as needed
    }
    
    // Most function keys should reset to normal mode after use
    if (field->kbd_mode == KB_MODE_2ND) {
        field->kbd_mode = KB_MODE_NORMAL;
    }
}

/**
 * Process a mode key (2nd, Alpha).
 * 
 * @param field Pointer to InputField structure
 * @param key The combined key code
 */
static void process_mode_key(InputField* field, CombinedKey key) {
    if (key == MAKE_KEY(1, kb_2nd)) {
        // 2nd key handling
        if (field->kbd_mode == KB_MODE_2ND) {
            // Pressing 2nd while in 2nd mode returns to normal
            field->kbd_mode = KB_MODE_NORMAL;
        } else if (field->kbd_mode == KB_MODE_ALPHA_LOCK || 
                  field->kbd_mode == KB_MODE_ALPHA_LOCK_LOWER) {
            // Exit alpha lock
            field->kbd_mode = KB_MODE_NORMAL;
        } else {
            // Enter 2nd mode
            field->kbd_mode = KB_MODE_2ND;
        }
    } else if (key == MAKE_KEY(2, kb_Alpha)) {
        // Alpha key handling
        if (field->kbd_mode == KB_MODE_2ND) {
            // 2nd+Alpha = Alpha Lock mode
            field->kbd_mode = KB_MODE_ALPHA_LOCK;
        } else if (field->kbd_mode == KB_MODE_ALPHA) {
            // Pressing Alpha again exits Alpha mode
            field->kbd_mode = KB_MODE_NORMAL;
        } else if (field->kbd_mode == KB_MODE_ALPHA_LOCK) {
            // Toggle between uppercase and lowercase in alpha lock
            field->kbd_mode = KB_MODE_ALPHA_LOCK_LOWER;
        } else if (field->kbd_mode == KB_MODE_ALPHA_LOCK_LOWER) {
            // Toggle back to uppercase in alpha lock
            field->kbd_mode = KB_MODE_ALPHA_LOCK;
        } else {
            // Enter Alpha mode
            field->kbd_mode = KB_MODE_ALPHA;
        }
    }
}

/**
 * Process a single key press.
 * 
 * @param field Pointer to InputField structure
 * @param key The combined key code
 * @return True if the key was processed, false if it caused an exit
 */
bool input_field_process_key(InputField* field, CombinedKey key) {
    // Handle special keys first
    if (key == MAKE_KEY(6, kb_Clear) || key == MAKE_KEY(6, kb_Enter)) {
        // These are handled by the calling function
        return false;
    } else if (key == MAKE_KEY(1, kb_Del)) {
        // Delete key is handled by the calling function
        return true;
    } else if (key == MAKE_KEY(7, kb_Left)) {
        // Special 2nd+Left = cursor to start
        if (field->kbd_mode == KB_MODE_2ND) {
            input_field_cursor_to_start(field);
            field->kbd_mode = KB_MODE_NORMAL;
        } else {
            input_field_cursor_left(field);
        }
        return true;
    } else if (key == MAKE_KEY(7, kb_Right)) {
        // Special 2nd+Right = cursor to end
        if (field->kbd_mode == KB_MODE_2ND) {
            input_field_cursor_to_end(field);
            field->kbd_mode = KB_MODE_NORMAL;
        } else {
            input_field_cursor_right(field);
        }
        return true;
    }
    
    // Handle mode keys (2nd, Alpha)
    if (key == MAKE_KEY(1, kb_2nd) || key == MAKE_KEY(2, kb_Alpha)) {
        process_mode_key(field, key);
        return true;
    }
    
    // Get the value based on current keyboard mode
    KeyboardState kb_state;
    kb_state.alpha_active = (field->kbd_mode == KB_MODE_ALPHA || 
                            field->kbd_mode == KB_MODE_ALPHA_LOCK || 
                            field->kbd_mode == KB_MODE_ALPHA_LOCK_LOWER);
    kb_state.second_active = (field->kbd_mode == KB_MODE_2ND);
    kb_state.alpha_lock = (field->kbd_mode == KB_MODE_ALPHA_LOCK || 
                          field->kbd_mode == KB_MODE_ALPHA_LOCK_LOWER);
    
    int value = key_mapping_get_value(key, kb_state);
    
    if (value >= 32 && value <= 126) {
        // For alpha lock lower, convert uppercase to lowercase
        if (field->kbd_mode == KB_MODE_ALPHA_LOCK_LOWER && value >= 'A' && value <= 'Z') {
            value = value + ('a' - 'A');  // Convert to lowercase
        }
        
        // Handle printable characters
        process_character_input(field, (char)value);
        
        // If in single alpha mode, return to normal
        if (field->kbd_mode == KB_MODE_ALPHA) {
            field->kbd_mode = KB_MODE_NORMAL;
        }
    } else if (value >= 128) {
        // Handle function keys
        process_function_key(field, value);
    }
    
    return true;
}

/**
 * Give focus to an input field and process input until focus is lost.
 * 
 * @param field Pointer to InputField structure
 * @return Result code indicating how focus was lost
 */
InputResult input_field_get_focus(InputField* field) {
    log_debug("input_field_get_focus() called");
    
    field->is_active = true;
    
    // Main input loop
    bool processing = true;
    InputResult result = INPUT_RESULT_CANCEL;
    
    // Variables for key repeat
    CombinedKey last_repeat_key = 0;
    unsigned long key_first_press_time = 0;
    unsigned long key_last_repeat_time = 0;
    bool key_in_repeat_mode = false;
    int initial_repeat_delay = 400;  // ms before first repeat
    int repeat_interval = 100;       // ms between repeats
    
    // Variables for key debounce
    CombinedKey last_key_processed = 0;
    unsigned long last_key_process_time = 0;
    int key_debounce_delay = 150;    // ms to wait before processing same key again (increased from 120ms)
    
    while (processing) {
        // Process keyboard events directly
        kb_Scan();
        
        // Draw the field
        input_field_draw(field);
        
        // Draw the keyboard mode indicator if callback is set
        if (field->mode_indicator_callback) {
            field->mode_indicator_callback(field->kbd_mode, field->x, field->y - 10);
        }
        
        // Render all changes to the screen
        gfx_BlitBuffer();
        
        // Current time for repeat and debounce logic
        unsigned long current_time = get_millis();
        
        // Check for key presses
        if (kb_AnyKey()) {
            // Get the first key that's pressed
            CombinedKey key = 0;
            
            // Scan through key groups
            for (int group = 1; group <= 7; group++) {
                uint8_t group_state = kb_Data[group];
                
                if (group_state) {
                    // Find the specific key bit
                    for (int bit = 0; bit < 8; bit++) {
                        uint8_t mask = 1 << bit;
                        if (group_state & mask) {
                            key = MAKE_KEY(group, mask);
                            break;
                        }
                    }
                    if (key) break;
                }
            }
            
            if (key) {
                // Check if this is a navigation key that should repeat
                bool is_repeat_key = (key == MAKE_KEY(7, kb_Left) || key == MAKE_KEY(7, kb_Right));
                bool should_process_key = false;
                
                if (is_repeat_key) {
                    // Handle repeating navigation keys
                    if (key != last_repeat_key) {
                        // New key, start repeat sequence
                        last_repeat_key = key;
                        key_first_press_time = current_time;
                        key_last_repeat_time = 0;
                        key_in_repeat_mode = false;
                        should_process_key = true;
                    } else {
                        // Same key still held down, check if we should repeat
                        if (!key_in_repeat_mode) {
                            // Check for initial delay
                            if (current_time - key_first_press_time >= (unsigned long)initial_repeat_delay) {
                                key_in_repeat_mode = true;
                                key_last_repeat_time = current_time;
                                should_process_key = true;
                            }
                        } else {
                            // In repeat mode, check for interval
                            if (current_time - key_last_repeat_time >= (unsigned long)repeat_interval) {
                                key_last_repeat_time = current_time;
                                should_process_key = true;
                            }
                        }
                    }
                } else {
                    // For non-repeating keys, use debounce
                    if (key != last_key_processed || 
                        (current_time - last_key_process_time) >= (unsigned long)key_debounce_delay) {
                        
                        // Reset repeat state since a different key was pressed
                        last_repeat_key = 0;
                        key_in_repeat_mode = false;
                        
                        should_process_key = true;
                        last_key_processed = key;
                        last_key_process_time = current_time;
                    }
                }
                
                // Process the key if needed
                if (should_process_key) {
                    // Handle special exit keys
                    if (key == MAKE_KEY(6, kb_Clear)) {
                        if (field->text_length > 0) {
                            input_field_clear(field);
                        } else {
                            result = INPUT_RESULT_CLEAR;
                            processing = false;
                        }
                    } 
                    else if (key == MAKE_KEY(6, kb_Enter)) {
                        if (field->next_field) {
                            result = INPUT_RESULT_NEXT;
                        } else {
                            result = INPUT_RESULT_ENTER;
                        }
                        processing = false;
                    }
                    else if (key == MAKE_KEY(1, kb_Del)) {
                        // Handle delete key
                        if (field->cursor_position == field->text_length) {
                            input_field_backspace(field);
                        } else {
                            input_field_delete(field);
                        }
                    }
                    else {
                        // Process other keys
                        input_field_process_key(field, key);
                    }
                }
            }
        } else {
            // No keys pressed, reset repeat and debounce state
            last_repeat_key = 0;
            key_in_repeat_mode = false;
            // We don't reset last_key_processed here to maintain debounce across key presses
        }
        
        // Small delay to reduce CPU usage
        delay(10);
    }
    
    field->is_active = false;
    return result;
}

// Remove the get_millis function since it's now externally visible
/* 
static unsigned long get_millis(void) {
    // On the TI-84 CE, timer_1_Counter runs at 32768 Hz
    // Convert to milliseconds (approximately)
    return (unsigned long)(timer_1_Counter / 32.768);
}
*/

/**
 * Register the next field to receive focus when Enter is pressed.
 * 
 * @param field Pointer to InputField structure
 * @param next_field The next field to receive focus
 */
void input_field_register_next(InputField* field, InputField* next_field) {
    field->next_field = next_field;
}

/**
 * Register a callback to draw the keyboard mode indicator.
 * 
 * @param field Pointer to InputField structure
 * @param callback Function to call to draw the mode indicator
 */
void input_field_register_mode_indicator(InputField* field, 
                                       void (*callback)(KeyboardMode mode, int x, int y)) {
    field->mode_indicator_callback = callback;
}

/**
 * Default implementation for drawing the keyboard mode indicator.
 * 
 * @param mode Current keyboard mode
 * @param x X position
 * @param y Y position
 */
static void draw_mode_indicator_default(KeyboardMode mode, int x, int y) {
    GUISettings* settings = GUI_get_settings();
    char* mode_text = "";
    
    switch (mode) {
        case KB_MODE_NORMAL:
            return; // No indicator for normal mode
        case KB_MODE_2ND:
            mode_text = "2ND";
            break;
        case KB_MODE_ALPHA:
            mode_text = "A";
            break;
        case KB_MODE_ALPHA_LOCK:
            mode_text = "A-LOCK";
            break;
        case KB_MODE_ALPHA_LOCK_LOWER:
            mode_text = "a-lock";
            break;
    }
    
    // Draw the mode indicator with inverted colors
    int text_width = gfx_GetStringWidth(mode_text);
    gfx_SetColor(settings->text_color);
    gfx_FillRectangle(x, y, text_width + 6, 10);
    
    // Draw the text in inverted color
    gfx_SetTextFGColor(settings->bg_color);
    gfx_SetTextBGColor(settings->text_color);
    gfx_PrintStringXY(mode_text, x + 3, y);
    
    // Reset text colors
    gfx_SetTextFGColor(settings->text_color);
    gfx_SetTextBGColor(settings->bg_color);
}

/**
 * Draw the input field.
 * 
 * @param field Pointer to InputField structure
 */
void input_field_draw(InputField* field) {
    GUISettings* settings = GUI_get_settings();
    
    // Draw background and border
    if (field->has_border) {
        // Draw border rectangle
        gfx_SetColor(settings->text_color);
        gfx_Rectangle(field->x, field->y, field->width, GUI_LINE_HEIGHT + 2*PADDING_Y);
        
        // Fill inner area
        gfx_SetColor(settings->bg_color);
        gfx_FillRectangle(field->x + 1, field->y + 1, 
                         field->width - 2, GUI_LINE_HEIGHT + 2*PADDING_Y - 2);
    } else {
        // Just fill the entire area
        gfx_SetColor(settings->bg_color);
        gfx_FillRectangle(field->x, field->y, field->width, GUI_LINE_HEIGHT + 2*PADDING_Y);
    }
    
    // Calculate visible area
    int content_x = field->x + PADDING_X;
    int content_y = field->y + PADDING_Y;
    
    // Create a temporary buffer for the visible text
    char visible_text[128]; // Assuming max_visible_chars is less than 128
    
    // Adjust visible area when scrolling
    int content_offset = 0;
    
    // Draw scroll indicators if needed
    if (field->scroll_offset > 0) {
        // Left scroll indicator as a small triangle
        gfx_SetColor(settings->text_color);
        int tri_x = field->x + 3;
        int tri_y = content_y + GUI_CHAR_HEIGHT/2;
        gfx_Line(tri_x + 4, tri_y - 3, tri_x, tri_y); // Top line of triangle
        gfx_Line(tri_x, tri_y, tri_x + 4, tri_y + 3); // Bottom line of triangle
        gfx_Line(tri_x + 4, tri_y - 3, tri_x + 4, tri_y + 3); // Vertical line to close triangle
        
        // Adjust content offset to make room for the indicator
        content_offset = GUI_CHAR_WIDTH;
    }
    
    // Extract visible portion of the text
    int visible_length = field->text_length - field->scroll_offset;
    int max_display_chars = (field->width - 2*PADDING_X - content_offset) / GUI_CHAR_WIDTH;
    if (visible_length > max_display_chars) {
        visible_length = max_display_chars;
    }
    
    if (visible_length > 0) {
        strncpy(visible_text, field->text + field->scroll_offset, visible_length);
    }
    visible_text[visible_length < 0 ? 0 : visible_length] = '\0';
    
    // Draw the visible text
    gfx_SetTextFGColor(settings->text_color);
    gfx_SetTextBGColor(settings->bg_color);
    gfx_PrintStringXY(visible_text, content_x + content_offset, content_y);
    
    // Draw cursor if active
    if (field->is_active) {
        int cursor_x = content_x + content_offset + (field->cursor_position - field->scroll_offset) * GUI_CHAR_WIDTH;
        gfx_SetColor(settings->text_color);
        gfx_Line(cursor_x, content_y, cursor_x, content_y + GUI_CHAR_HEIGHT);
    }
    
    if (field->scroll_offset + max_display_chars < field->text_length) {
        // Right scroll indicator as a small triangle
        gfx_SetColor(settings->text_color);
        int tri_x = field->x + field->width - 8;
        int tri_y = content_y + GUI_CHAR_HEIGHT/2;
        gfx_Line(tri_x, tri_y - 3, tri_x + 4, tri_y); // Top line of triangle
        gfx_Line(tri_x + 4, tri_y, tri_x, tri_y + 3); // Bottom line of triangle
        gfx_Line(tri_x, tri_y - 3, tri_x, tri_y + 3); // Vertical line to close triangle
    }
}
