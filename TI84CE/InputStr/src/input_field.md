# Input Field Component Specification

## Overview
The Input Field component provides a self-contained, interactive text input field for the TI-84 CE calculator. It handles all keyboard input processing, supports multiple keyboard modes (Normal, 2nd, Alpha, Alpha Lock, Alpha Lock Lower), and provides a clean API for applications to use.

## Architecture
The Input Field builds upon the existing horizontal scrolling text field implementation but adds enhanced keyboard handling capabilities.

### Components:
1. **InputField**: Core component providing text input functionality
2. **Keyboard Mode Management**: Handling Normal, 2nd, Alpha, and Alpha Lock modes
3. **Field Navigation**: Support for chaining multiple fields together

## Data Structures

### InputField Structure
```c
typedef struct InputField {
    // Display properties
    int x, y;                // Position on screen
    int width;               // Width of the field in pixels
    bool has_border;         // Whether to draw a border
    
    // Text properties
    char* text;              // Text content
    int text_length;         // Length of the text
    int buffer_size;         // Total buffer size allocated
    int cursor_position;     // Cursor position in the text (character index)
    int scroll_offset;       // Horizontal scroll offset (character index)
    int max_visible_chars;   // Maximum number of visible characters
    
    // State
    bool is_active;          // Whether this field is currently active/focused
    KeyboardMode kbd_mode;   // Current keyboard mode
    
    // Callbacks
    void (*mode_indicator_callback)(KeyboardMode mode, int x, int y); // Callback to draw mode indicator
    
    // Linking
    struct InputField* next_field; // Next field in the chain (for Enter key)
} InputField;
```

### Keyboard Mode Enumeration
```c
typedef enum {
    KB_MODE_NORMAL,        // Normal mode (numbers, operators)
    KB_MODE_2ND,           // 2nd mode (blue functions)
    KB_MODE_ALPHA,         // Alpha mode (single uppercase letter)
    KB_MODE_ALPHA_LOCK,    // Alpha Lock mode (multiple uppercase letters)
    KB_MODE_ALPHA_LOCK_LOWER // Alpha Lock Lower mode (lowercase letters)
} KeyboardMode;
```

### Result Enumeration
```c
typedef enum {
    INPUT_RESULT_ENTER,    // Enter key was pressed (submission)
    INPUT_RESULT_CLEAR,    // Clear key was pressed on empty field
    INPUT_RESULT_NEXT,     // Focus moved to next field
    INPUT_RESULT_CANCEL    // Operation was canceled
} InputResult;
```

## API Functions

### Core Functions

#### Initialization and Management
```c
// Initialize an input field
void input_field_init(InputField* field, int x, int y, int width, bool has_border);

// Free resources used by an input field
void input_field_free(InputField* field);

// Clear all text from the field
void input_field_clear(InputField* field);
```

#### Text Operations
```c
// Set the text content
void input_field_set_text(InputField* field, const char* text);

// Get the current text content
const char* input_field_get_text(InputField* field);

// Append text to the current content
void input_field_append(InputField* field, const char* text);
```

#### Input Processing
```c
// Give focus to an input field and process input until focus is lost
// Returns how focus was lost (Enter, Clear, etc.)
InputResult input_field_get_focus(InputField* field);

// Process a single key press (for more manual control)
bool input_field_process_key(InputField* field, CombinedKey key);
```

#### Drawing
```c
// Draw the input field (called automatically by get_focus, or manually if needed)
void input_field_draw(InputField* field);
```

#### Navigation
```c
// Register the next field to receive focus when Enter is pressed
void input_field_register_next(InputField* field, InputField* next_field);

// Register a callback to draw the keyboard mode indicator
void input_field_register_mode_indicator(InputField* field, 
                                         void (*callback)(KeyboardMode mode, int x, int y));
```

### Cursor and Selection

```c
// Move cursor left one character
void input_field_cursor_left(InputField* field);

// Move cursor right one character
void input_field_cursor_right(InputField* field);

// Move cursor to the start of the text
void input_field_cursor_to_start(InputField* field);

// Move cursor to the end of the text
void input_field_cursor_to_end(InputField* field);
```

### Internal Functions (Not exposed in API)

```c
// Ensure the buffer is large enough for the given size
static bool ensure_buffer_size(InputField* field, int needed_size);

// Ensure the cursor is visible in the current scroll view
static void ensure_cursor_visible(InputField* field);

// Process a character input in the current mode
static void process_character_input(InputField* field, char c);

// Process a function key input in the current mode
static void process_function_key(InputField* field, int func_code);

// Process a mode key (2nd, Alpha)
static void process_mode_key(InputField* field, CombinedKey key);
```

## Keyboard Handling

### Mode Transitions

1. **Normal Mode**:
   - 2nd key → 2nd Mode
   - Alpha key → Alpha Mode

2. **2nd Mode**:
   - Alpha key → Alpha Lock Mode 
   - Any other key → process with 2nd function, then return to Normal Mode
   - Special case: 2nd+LEFT → cursor to start
   - Special case: 2nd+RIGHT → cursor to end

3. **Alpha Mode**:
   - Process next key as uppercase letter, then return to Normal Mode
   - Alpha key again → Alpha Mode for lowercase (next key only)

4. **Alpha Lock Mode**:
   - Produce uppercase letters until mode changes
   - Alpha key → Alpha Lock Lower Mode
   - 2nd key → Normal Mode

5. **Alpha Lock Lower Mode**:
   - Produce lowercase letters until mode changes
   - Alpha key → Alpha Lock Mode
   - 2nd key → Normal Mode

### Special Key Handling

- **Enter**: Submit the field or move to next field if registered
- **Clear**: 
  - If field has text, clear the text
  - If field is empty, return control to caller
- **Delete**: Delete character before cursor (backspace)
- **Cursor Keys**: Move cursor left/right

## Usage Example

```c
void main() {
    // Setup
    GUI_init();
    
    // Create input fields
    InputField name_field, formula_field;
    input_field_init(&name_field, 10, 30, LCD_WIDTH - 20, true);
    input_field_init(&formula_field, 10, 80, LCD_WIDTH - 20, true);
    
    // Chain fields
    input_field_register_next(&name_field, &formula_field);
    
    // Set initial text
    input_field_set_text(&name_field, "");
    input_field_set_text(&formula_field, "sin(x) + 5");
    
    // Draw labels
    GUI_write_text(10, 20, "Name:");
    GUI_write_text(10, 70, "Formula:");
    
    // Get input
    InputResult result = input_field_get_focus(&name_field);
    
    // Process input
    if (result == INPUT_RESULT_CLEAR) {
        // User canceled
        GUI_write_text_centered(100, "Operation canceled");
    } else {
        // Process the inputs
        char* name = input_field_get_text(&name_field);
        char* formula = input_field_get_text(&formula_field);
        
        // Do something with the inputs...
    }
    
    // Cleanup
    input_field_free(&name_field);
    input_field_free(&formula_field);
    GUI_end();
}
```

## Implementation Notes

1. The InputField manages its own memory for text content
2. When a field has focus, it handles all keyboard input completely
3. Visual indicators should show the current keyboard mode
4. The field returns control to the caller only when an exit condition is met

## Future Enhancements

1. Selection support (highlighting text sections)
2. Cut/Copy/Paste functionality
3. Context-sensitive suggestions for math formulas
4. Validation functions (e.g., ensuring valid math expressions)
5. Additional visual styles and theming