/**
 * Input Field Component for TI-84 CE
 * 
 * Provides a self-contained, interactive text input field that handles all
 * keyboard input processing and supports multiple keyboard modes.
 */

#ifndef INPUT_FIELD_H
#define INPUT_FIELD_H

#include <stdbool.h>
#include "kb_handler.h"
#include "kb_mapping.h"

/**
 * Result code for input field operations.
 */
typedef enum {
    INPUT_RESULT_ENTER,       // Enter key was pressed (submission)
    INPUT_RESULT_CLEAR,       // Clear key was pressed on empty field
    INPUT_RESULT_NEXT,        // Focus moved to next field
    INPUT_RESULT_CANCEL       // Operation was canceled
} InputResult;

// Forward declaration for self-referencing structure
typedef struct InputField InputField;

/**
 * Input field structure.
 */
struct InputField {
    // Display properties
    int x, y;                 // Position on screen
    int width;                // Width of the field in pixels
    bool has_border;          // Whether to draw a border
    
    // Text properties
    char* text;               // Text content
    int text_length;          // Length of the text
    int buffer_size;          // Total buffer size allocated
    int cursor_position;      // Cursor position in the text (character index)
    int scroll_offset;        // Horizontal scroll offset (character index)
    int max_visible_chars;    // Maximum number of visible characters
    
    // State
    bool is_active;           // Whether this field is currently active/focused
    KeyboardMode kbd_mode;    // Current keyboard mode
    
    // Callbacks
    void (*mode_indicator_callback)(KeyboardMode mode, int x, int y); // Callback to draw mode indicator
    
    // Linking
    InputField* next_field;   // Next field in the chain (for Enter key)
};

#include "input_field_public.h"

#endif /* INPUT_FIELD_H */
