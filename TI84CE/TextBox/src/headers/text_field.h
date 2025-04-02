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
    TEXT_RESULT_ENTER,       /**< Enter key was pressed (submission) */
    TEXT_RESULT_CLEAR,       /**< Clear key was pressed on empty field */
    TEXT_RESULT_NEXT,        /**< Focus moved to next field */
    TEXT_RESULT_PREV,        /**< Focus moved to previous field */
    TEXT_RESULT_CANCEL       /**< Operation was canceled */
} TextResult;

/**
 * Forward declaration for self-referencing structure.
 */
typedef struct TextField TextField;

/**
 * Text field structure.
 */
struct TextField {
    // Display properties

    /** X-coordinate of the text field on the screen. */
    int x;

    /** Y-coordinate of the text field on the screen. */
    int y;

    /** Width of the text field in pixels. */
    int width;

    /** Height of the text field in pixels. */
    int height;

    /** Whether to draw a border around the text field. */
    bool has_border;

    /** Horizontal padding inside the text field. */
    int padding_x;

    /** Vertical padding inside the text field. */
    int padding_y;

    // Text properties

    /** Pointer to the text content of the field. */
    char* text;

    /** Current length of the text in the field. */
    int text_length;

    /** Total buffer size allocated for the text. */
    int buffer_size;

    /** Cursor position in the text (character index). */
    int cursor_position;

    /** Horizontal scroll offset (character index). */
    int scroll_offset;

    /** Maximum number of visible characters in the field. */
    int max_visible_chars;

    /** Whether the text field is in password mode (hides text). */
    bool password_mode;

    /** Character to display for hidden text in password mode. */
    char password_char;

    // State

    /** Whether the text field is currently active or focused. */
    bool is_active;

    /** Whether the text field is read-only. */
    bool read_only;

    // Callbacks

    /**
     * Callback to draw the mode indicator.
     * 
     * @param mode The current keyboard mode.
     * @param x The x-coordinate for the mode indicator.
     * @param y The y-coordinate for the mode indicator.
     */
    void (*mode_indicator_callback)(KeyboardMode mode, int x, int y);

    /**
     * Callback invoked when the text in the field changes.
     * 
     * @param field Pointer to the text field instance.
     */
    void (*on_changed)(TextField* field);

    /**
     * Callback invoked when the Enter key is pressed.
     * 
     * @param field Pointer to the text field instance.
     */
    void (*on_enter)(TextField* field);

    // Linking

    /** Pointer to the next text field in the chain (for Enter key navigation). */
    TextField* next_field;

    /** Pointer to the previous text field in the chain (for Shift+Tab navigation). */
    TextField* prev_field;
};

#include "text_field_public.h"

#endif // TEXT_FIELD_H
