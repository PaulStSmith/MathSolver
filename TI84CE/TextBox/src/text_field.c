#include <tice.h>
#include <graphx.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "headers/keyboard.h"
#include "headers/key_translator.h"
#include "headers/text_field_private.h"
#include "headers/gui.h"

#include "headers/log.h"

// Default initial buffer size
#define INITIAL_BUFFER_SIZE 64

/**
 * Initialize a text field.
 */
void text_field_init(TextField* field, int x, int y, int width, bool has_border) {
    if (!field) return;
    log_message("Initializing text field at (%d, %d) with width %d", x, y, width);
    
    memset(field, 0, sizeof(TextField));
    field->x = x;
    field->y = y;
    field->width = width;
    field->height = GUI_LINE_HEIGHT + 2*PADDING_Y;
    field->has_border = has_border;
    field->padding_x = PADDING_X;
    field->padding_y = PADDING_Y;
    
    // Calculate maximum visible characters
    field->max_visible_chars = (width - 2*PADDING_X) / GUI_CHAR_WIDTH;
    
    // Allocate initial buffer
    field->buffer_size = INITIAL_BUFFER_SIZE;
    field->text = (char*)malloc(field->buffer_size);
    if (field->text) {
        field->text[0] = '\0';
    }
    
    // Initialize other values
    field->text_length = 0;
    field->cursor_position = 0;
    field->scroll_offset = 0;
    field->is_active = false;
    field->read_only = false;
    field->password_mode = false;
    field->password_char = '*';
    field->has_selection = false;
    field->selection_start = 0;
    field->selection_end = 0;
    field->next_field = NULL;
    field->prev_field = NULL;
    field->on_changed = NULL;
    field->on_enter = NULL;
    
    // Set default mode indicator callback
    field->mode_indicator_callback = draw_mode_indicator_default;
    log_message("Text field initialized successfully");
}

/**
 * Free resources used by a text field.
 */
void text_field_free(TextField* field) {
    if (!field) return;
    log_message("Freeing text field resources");
    
    if (field->text) {
        free(field->text);
        field->text = NULL;
    }
    field->buffer_size = 0;
    field->text_length = 0;
    log_message("Text field resources freed");
}

/**
 * Ensure the buffer is large enough for the given size.
 */
static bool ensure_buffer_size(TextField* field, int needed_size) {
    if (!field || !field->text) return false;
    log_message("Ensuring buffer size: needed %d, current %d", needed_size, field->buffer_size);
    
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
        return false;
    }
    
    field->text = new_buffer;
    field->buffer_size = new_size;
    log_message("Buffer size ensured: new size %d", field->buffer_size);
    return true;
}

/**
 * Clear all text from the field.
 */
void text_field_clear(TextField* field) {
    if (!field || !field->text) return;
    log_message("Clearing text field");
    
    field->text[0] = '\0';
    field->text_length = 0;
    field->cursor_position = 0;
    field->scroll_offset = 0;
    field->has_selection = false;
    field->selection_start = 0;
    field->selection_end = 0;
    
    // Trigger on_changed callback if registered
    if (field->on_changed) {
        field->on_changed(field);
    }
    log_message("Text field cleared");
}

/**
 * Set the text content of the field.
 */
void text_field_set_text(TextField* field, const char* text) {
    if (!field || !text) return;
    log_message("Setting text field content: \"%s\"", text);
    
    int length = strlen(text);
    if (ensure_buffer_size(field, length + 1)) {
        strcpy(field->text, text);
        field->text_length = length;
        
        // Reset cursor and scroll
        field->cursor_position = length;
        field->has_selection = false;
        field->selection_start = 0;
        field->selection_end = 0;
        ensure_cursor_visible(field);
        
        // Trigger on_changed callback if registered
        if (field->on_changed) {
            field->on_changed(field);
        }
    }
    log_message("Text field content set successfully");
}

/**
 * Get the current text content of the field.
 */
const char* text_field_get_text(TextField* field) {
    if (!field || !field->text) return "";
    log_message("Getting text field content");
    return field->text;
}

/**
 * Set the field to read-only mode.
 */
void text_field_set_read_only(TextField* field, bool read_only) {
    if (!field) return;
    log_message("Setting text field read-only mode to %s", read_only ? "true" : "false");
    field->read_only = read_only;
}

/**
 * Enable or disable password mode.
 */
void text_field_set_password_mode(TextField* field, bool password_mode, char password_char) {
    if (!field) return;
    log_message("Setting password mode to %s with char '%c'", password_mode ? "enabled" : "disabled", password_char);
    field->password_mode = password_mode;
    field->password_char = password_char;
}

/**
 * Register the next field to receive focus when Enter is pressed.
 */
void text_field_register_next(TextField* field, TextField* next_field) {
    if (!field) return;
    field->next_field = next_field;
}

/**
 * Register the previous field to receive focus when Shift+Tab is pressed.
 */
void text_field_register_prev(TextField* field, TextField* prev_field) {
    if (!field) return;
    field->prev_field = prev_field;
}

/**
 * Register a callback for when the field text changes.
 */
void text_field_on_changed(TextField* field, void (*callback)(TextField* field)) {
    if (!field) return;
    field->on_changed = callback;
}

/**
 * Register a callback for when Enter is pressed in the field.
 */
void text_field_on_enter(TextField* field, void (*callback)(TextField* field)) {
    if (!field) return;
    field->on_enter = callback;
}

/**
 * Register a callback to draw the keyboard mode indicator.
 */
void text_field_register_mode_indicator(TextField* field, 
                                        void (*callback)(KeyboardMode mode, int x, int y)) {
    if (!field) return;
    field->mode_indicator_callback = callback;
}

/**
 * Insert a character at the current cursor position.
 */
static void text_field_insert_char(TextField* field, char c) {
    if (!field || field->read_only) return;
    log_message("Inserting character '%c' at position %d", c, field->cursor_position);
    
    // If we have a selection, delete it first
    if (field->has_selection) {
        // TODO: Implement selection deletion
        field->has_selection = false;
    }
    
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
        
        // Trigger on_changed callback if registered
        if (field->on_changed) {
            field->on_changed(field);
        }
    }
    log_message("Character inserted successfully");
}

/**
 * Delete the character before the cursor (backspace).
 */
static void text_field_backspace(TextField* field) {
    if (!field || field->read_only) return;
    log_message("Performing backspace at position %d", field->cursor_position);
    
    // If we have a selection, delete it
    if (field->has_selection) {
        // TODO: Implement selection deletion
        field->has_selection = false;
        return;
    }
    
    if (field->cursor_position > 0) {
        // Shift characters at and after cursor
        for (int i = field->cursor_position - 1; i < field->text_length; i++) {
            field->text[i] = field->text[i + 1];
        }
        
        field->text_length--;
        field->cursor_position--;
        
        ensure_cursor_visible(field);
        
        // Trigger on_changed callback if registered
        if (field->on_changed) {
            field->on_changed(field);
        }
    }
    log_message("Backspace completed");
}

/**
 * Delete the character at the cursor position.
 */
static void text_field_delete(TextField* field) {
    if (!field || field->read_only) return;
    log_message("Deleting character at position %d", field->cursor_position);
    
    // If we have a selection, delete it
    if (field->has_selection) {
        // TODO: Implement selection deletion
        field->has_selection = false;
        return;
    }
    
    // Only proceed if we're not at the end of the text
    if (field->cursor_position < field->text_length) {
        // Shift characters after cursor
        for (int i = field->cursor_position; i < field->text_length; i++) {
            field->text[i] = field->text[i + 1];
        }
        
        field->text_length--;
        
        // Cursor position stays the same
        ensure_cursor_visible(field);
        
        // Trigger on_changed callback if registered
        if (field->on_changed) {
            field->on_changed(field);
        }
    }
    log_message("Character deleted");
}

/**
 * Ensure the cursor is visible in the current scroll view.
 */
static void ensure_cursor_visible(TextField* field) {
    if (!field) return;
    
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
 * Process a navigation key.
 */
static void process_navigation(TextField* field, int value) {
    if (!field) return;
    
    switch (value) {
        case CHAR_LEFT:
            if (field->cursor_position > 0) {
                field->cursor_position--;
                field->has_selection = false;
                ensure_cursor_visible(field);
            }
            break;
            
        case CHAR_RIGHT:
            if (field->cursor_position < field->text_length) {
                field->cursor_position++;
                field->has_selection = false;
                ensure_cursor_visible(field);
            }
            break;
            
        case CHAR_HOME:
            field->cursor_position = 0;
            field->scroll_offset = 0;
            field->has_selection = false;
            break;
            
        case CHAR_END:
            field->cursor_position = field->text_length;
            field->has_selection = false;
            ensure_cursor_visible(field);
            break;
            
        // Add more navigation cases as needed
    }
}

/**
 * Process a character input.
 */
static void process_character_input(TextField* field, int value) {
    if (!field || field->read_only) return;
    log_message("Processing character input: %d", value);
    
    // Handle printable ASCII characters
    if (value >= 32 && value <= 126) {
        text_field_insert_char(field, (char)value);
    }
    // Handle function key input (math functions, etc.)
    else if (value >= 128) {
        // TODO: Handle special function keys for math input
    }
}

/**
 * Default implementation for drawing the keyboard mode indicator.
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
        case KB_MODE_ALPHA_LOWER:
            mode_text = "a";
            break;
        case KB_MODE_ALPHA_LOWER_LOCK:
            mode_text = "a-lock";
            break;
        default:
            mode_text = "??"; // should not happen
            break;
    }
    
    // Draw the mode indicator with inverted colors
    int text_width = gfx_GetStringWidth(mode_text);
    gfx_SetColor(settings->text_color);
    gfx_FillRectangle(x, y, text_width + 6, 10);
    
    // Use GUI functions to set and reset text colors
    GUI_set_text_colors(settings->bg_color, settings->text_color);
    GUI_write_text(x + 3, y, mode_text);
    GUI_reset_text_colors();
}

/**
 * Draw the text field.
 */
void text_field_draw(TextField* field) {
    if (!field) return;
    log_message("Drawing text field at (%d, %d)", field->x, field->y);
    
    GUISettings* settings = GUI_get_settings();
    
    // Draw background and border
    if (field->has_border) {
        // Draw border rectangle
        gfx_SetColor(settings->text_color);
        gfx_Rectangle(field->x, field->y, field->width, field->height);
        
        // Fill inner area
        gfx_SetColor(settings->bg_color);
        gfx_FillRectangle(field->x + 1, field->y + 1, 
                         field->width - 2, field->height - 2);
    } else {
        // Just fill the entire area
        gfx_SetColor(settings->bg_color);
        gfx_FillRectangle(field->x, field->y, field->width, field->height);
    }
    
    // Calculate visible area
    int content_x = field->x + field->padding_x;
    int content_y = field->y + field->padding_y;
    
    // Create a temporary buffer for the visible text
    char visible_text[128]; // Assuming max_visible_chars is less than 128
    memset(visible_text, 0, sizeof(visible_text));
    
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
    int max_display_chars = (field->width - 2*field->padding_x - content_offset) / GUI_CHAR_WIDTH;
    if (visible_length > max_display_chars) {
        visible_length = max_display_chars;
    }
    
    if (visible_length > 0 && field->text) {
        if (field->password_mode) {
            // In password mode, fill with password char
            for (int i = 0; i < visible_length; i++) {
                visible_text[i] = field->password_char;
            }
            visible_text[visible_length] = '\0';
        } else {
            // Normal text display
            strncpy(visible_text, field->text + field->scroll_offset, visible_length);
            visible_text[visible_length] = '\0';
        }
    }
    
    // Draw the visible text
    GUI_reset_text_colors();
    GUI_write_text(content_x + content_offset, content_y, visible_text);
    
    // Draw cursor if active
    if (field->is_active) {
        int cursor_x = content_x + content_offset + 
                      (field->cursor_position - field->scroll_offset) * GUI_CHAR_WIDTH;
        gfx_SetColor(settings->text_color);
        gfx_Line(cursor_x, content_y, cursor_x, content_y + GUI_CHAR_HEIGHT);
    }
    
    // Draw right scroll indicator if needed
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

static void on_key_press(void* sender, int value) {
    log_message("on_key_press_callback: Key press event for value %d", value);

    TextField* field = (TextField*)sender;
    
    if (!field) return;
    
    // Handle repeatable actions
    switch (value) {
        case CHAR_LEFT:
        case CHAR_RIGHT:
        case CHAR_HOME:
        case CHAR_END:
        case CHAR_UP:
        case CHAR_DOWN:
            process_navigation(field, value);
            break;
            
        case CHAR_DEL:
            if (!field->read_only) {
                if (field->cursor_position == field->text_length) {
                    text_field_backspace(field);
                } else {
                    text_field_delete(field);
                }
            }
            break;
            
        // Process regular character input
        default:
            process_character_input(field, value);
    }
    
    // Redraw the field
    text_field_draw(field);
    if (field->mode_indicator_callback) {
        field->mode_indicator_callback(char_get_mode(), 
            field->x, field->y - 12);
    }
    GUI_refresh();
}

/**
 * Give focus to a text field and process input until focus is lost.
 * This now uses the procedural char_get_char() approach.
 */
TextResult text_field_activate(TextField* field) {
    if (!field) return TEXT_RESULT_CANCEL;
    log_message("Text field gaining focus");
    
    // Initialize character handling
    char_init(field);
    
    // Register for key press callback for repeats
    int text_field_callback_id = char_register_press(field, on_key_press, 500, 100);
    
    // Mark field as active
    field->is_active = true;
    
    // Draw the field initially
    text_field_draw(field);
    if (field->mode_indicator_callback) {
        field->mode_indicator_callback(char_get_mode(), field->x, field->y - 12);
    }
    GUI_refresh();
    
    // Process input until done
    TextResult result = TEXT_RESULT_CANCEL;
    bool processing = true;
    
    while (processing) {
        // Get the next character input (this blocks until a key is pressed)
        // The key_wait inside char_get_char will trigger our callback for repeats
        int value = char_get_char(field);
        
        // Handle non-repeating actions here
        switch (value) {
            case CHAR_ENTER:
                if (field->on_enter) {
                    field->on_enter(field);
                }
                
                if (field->next_field) {
                    result = TEXT_RESULT_NEXT;
                } else {
                    result = TEXT_RESULT_ENTER;
                }
                processing = false;
                break;
                
            case CHAR_CLEAR:
                if (!field->read_only && field->text_length > 0) {
                    text_field_clear(field);
                } else if (field->text_length == 0) {
                    // Exit if text field is already empty and Clear is pressed
                    result = TEXT_RESULT_CLEAR;
                    processing = false;
                }
                break;
                
            default:
                break;
        }
        
        // Redraw the field after each key press
        text_field_draw(field);
        if (field->mode_indicator_callback) {
            field->mode_indicator_callback(char_get_mode(), field->x, field->y - 12);
        }
        GUI_refresh();
    }
    
    // Clean up
    if (text_field_callback_id >= 0) {
        char_unregister(text_field_callback_id);
        text_field_callback_id = -1;
    }
    
    // Mark field as inactive when done
    field->is_active = false;
    
    // Final redraw to show inactive state
    text_field_draw(field);
    GUI_refresh();
    char_deinit(); // Clean up char subsystem
    field = NULL; // Clear reference to field
    
    log_message("Text field focus processing completed with result %d", result);
    return result;
}
