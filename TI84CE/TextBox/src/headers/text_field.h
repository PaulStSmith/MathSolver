/**
 * Text Field Component for TI-84 CE (Layer 3)
 * 
 * Provides a self-contained, interactive text input field that handles all
 * keyboard input processing and supports multiple keyboard modes. This builds
 * on Layer 2 (Key Translator) to provide a complete text editing component.
 */

#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include <stdbool.h>
#include "keyboard.h"
#include "key_translator.h"

/**
 * Result code for text field operations.
 */
typedef enum {
    TEXT_RESULT_ENTER,       // Enter key was pressed (submission)
    TEXT_RESULT_CLEAR,       // Clear key was pressed on empty field
    TEXT_RESULT_NEXT,        // Focus moved to next field
    TEXT_RESULT_PREV,        // Focus moved to previous field
    TEXT_RESULT_CANCEL       // Operation was canceled
} TextResult;

// Forward declaration for self-referencing structure
typedef struct TextField TextField;

/**
 * Text field structure.
 */
struct TextField {
    // Display properties
    int x, y;                 // Position on screen
    int width;                // Width of the field in pixels
    int height;               // Height of the field in pixels
    bool has_border;          // Whether to draw a border
    int padding_x, padding_y; // Padding inside the field
    
    // Text properties
    char* text;               // Text content
    int text_length;          // Length of the text
    int buffer_size;          // Total buffer size allocated
    int cursor_position;      // Cursor position in the text (character index)
    int scroll_offset;        // Horizontal scroll offset (character index)
    int max_visible_chars;    // Maximum number of visible characters
    bool password_mode;       // Whether to hide text (password input)
    char password_char;       // Character to show for passwords
    
    // Selection properties
    bool has_selection;       // Whether text is currently selected
    int selection_start;      // Start position of selection
    int selection_end;        // End position of selection
    
    // State
    bool is_active;           // Whether this field is currently active/focused
    bool read_only;           // Whether the field is read-only
    
    // Callbacks
    void (*mode_indicator_callback)(KeyboardMode mode, int x, int y); // Callback to draw mode indicator
    void (*on_changed)(TextField* field);   // Called when text changes
    void (*on_enter)(TextField* field);     // Called when Enter pressed
    
    // Linking
    TextField* next_field;    // Next field in the chain (for Enter key)
    TextField* prev_field;    // Previous field in the chain (for Shift+Tab)
};

#include "text_field_public.h"

#endif // TEXT_FIELD_H
