# Keyboard Architecture

This document outlines the keyboard handling architecture for the TI-84 CE calculator application framework. The system is designed with a layered approach to separate concerns and provide flexibility for different types of applications.

## Architecture Overview

The keyboard handling system consists of three distinct layers:

1. **Keyboard** - Low-level hardware interface
2. **Key Translator** - Mode management and key interpretation
3. **Text Field** - High-level text input component

Each layer builds upon the functionality of the layers below it and provides progressively higher-level abstractions for handling keyboard input.

```
+------------------+
|    Text Field    | High-level UI component
+------------------+
|  Key Translator  | Logical key mapping
+------------------+
|     Keyboard     | Physical key detection
+------------------+
|  TI-84 Hardware  |
+------------------+
```

## Platform Constraints

The TI-84 CE has important constraints that influence the keyboard architecture:

- Single-threaded processor
- No access to processor interrupts
- Limited memory and processing power

These constraints require a sequential, polling-based approach rather than true event-driven programming.

## Layer 1: Keyboard (`keyboard.c/h`)

The Keyboard layer is responsible for direct interaction with the calculator's hardware keyboard. It handles physical key detection and provides a blocking interface that handles a complete key press cycle.

### Responsibilities:
- Scan the keyboard hardware
- Detect key press, repeat, and release events
- Handle debouncing to avoid unintended duplicate key presses
- Manage key repetition timing
- Invoke registered callbacks for key state changes
- Wait for complete key press cycles

### Key Functions:
- `key_wait_press` - Blocks until a key is pressed and released, triggers registered callbacks
- `key_register_down` - Register callback for key down events
- `key_register_press` - Register callback for key press events (repeating)
- `key_register_up` - Register callback for key up events

### Example Usage:
```c
// Register callbacks for key events
key_register_down(on_key_down);  // void on_key_down(int group, int mask)
key_register_press(on_key_press); // void on_key_press(int group, int mask)
key_register_up(on_key_up);  // void on_key_up(int group, int mask)

// Wait for a complete key press cycle, callbacks will be triggered
key_wait_press();
```

## Layer 2: Key Translator (`key_translator.c/h`)

The Key Translator layer manages keyboard modes (Normal, 2nd, Alpha, etc.) and translates physical key events into logical values based on the current mode. This provides a higher-level abstraction that deals with characters and calculator functions rather than raw key codes.

### Responsibilities:
- Manage keyboard modes (Normal, 2nd, Alpha, Alpha-Lock, etc.)
- Translate physical key codes to logical values (characters, function codes)
- Process mode keys (2nd, Alpha) to change keyboard state
- Provide a sequential interface for character input

### Key Functions:
- `char_wait_press` - Blocks until a character input is complete, triggers registered callbacks
- `char_on_key_down` - Internal callback, handles key down from keyboard layer
- `char_on_key_press` - Internal callback, handles key press from keyboard layer
- `char_on_key_up` - Internal callback, handles key up from keyboard layer
- `char_register_down` - Register callback for character down events
- `char_register_press` - Register callback for character press events
- `char_register_up` - Register callback for character up events

### Example Usage:
```c
// Register callbacks for character events
char_register_down(on_char_down);  // void on_char_down(char value)
char_register_press(on_char_press);  // void on_char_press(char value)
char_register_up(on_char_up);  // void on_char_up(char value)

// Wait for a complete character input cycle, callbacks will be triggered
char_wait_press();
```

## Layer 3: Text Field (`text_field.c/h`)

The Text Field layer provides a complete text input component that handles text editing, display, and user interaction. It uses the Key Translator layer to obtain character input and implements all the logic needed for a fully functional text input field.

### Responsibilities:
- Manage text buffer and cursor position
- Handle text editing operations (insert, delete, etc.)
- Process navigation keys (arrows, home, end, etc.)
- Manage text selection
- Handle scrolling for long text
- Render the text field and cursor on screen
- Return input results to the application

### Key Functions:
- `text_field_init` - Initialize a text field with given parameters
- `text_field_get_focus` - Process input until completion (Enter, Clear, etc.)
- `text_field_set_text` - Set the content of the text field
- `text_field_get_text` - Get the current content of the text field
- `text_field_draw` - Render the text field on screen

### Example Usage:
```c
// Create and initialize a text field
TextField input_field;
text_field_init(&input_field, 10, 30, 100, true);

// Give focus to the field and process input
InputResult result = text_field_get_focus(&input_field);

// Check the result
if (result == INPUT_RESULT_ENTER) {
    // Process the entered text
    const char* input_text = text_field_get_text(&input_field);
    // ...
}
```

## Processing Flow

Rather than true event-driven programming, the architecture uses nested blocking calls with callbacks:

1. The application calls `text_field_get_focus()`
2. The text field registers callbacks with the key translator for character events
3. The key translator registers its callbacks with the keyboard layer
4. The text field calls `char_wait_press()` to wait for character input
5. The key translator calls `key_wait_press()` to wait for physical key input
6. The keyboard layer scans the hardware until a key cycle completes, triggering callbacks
7. The key translator's callbacks translate keys to characters and trigger character callbacks
8. The text field's callbacks process the character input
9. This continues until a special condition occurs (Enter pressed, Clear on empty field, etc.)

This sequential approach fits the single-threaded nature of the TI-84 CE platform while still maintaining separation of concerns through the layered architecture.

## Application Use Cases

Different types of applications can utilize this architecture in different ways:

### Games
- May bypass the Key Translator and Text Field layers entirely
- Use the Keyboard layer directly for low-latency input
- Example: 
```c
// Game loop
while (game_running) {
    int group, mask;
    if (key_is_pressed(KEY_UP)) {
        move_player_up();
    }
    // Process game logic
    // Render graphics
    delay(16); // ~60 FPS
}
```

### Command-Line Tools
- Might use the Key Translator layer for character-based input
- Process one character at a time via callbacks
- Example:
```c
char cmd_buffer[64] = {0};
int pos = 0;

// Callback for character input
void on_char_down(char c) {
    if (c == '\n') {
        // Process command
        process_command(cmd_buffer);
        pos = 0;
        cmd_buffer[0] = 0;
    } else if (c != 0) {
        cmd_buffer[pos++] = c;
        cmd_buffer[pos] = 0;
    }
}

// Main loop
while (running) {
    char_register_down(on_char_down);
    char_wait_press();
}
```

### Form-Based Applications
- Would use the Text Field component for user input
- Chain multiple fields together
- Example:
```c
TextField name_field, email_field;
text_field_init(&name_field, 10, 30, 100, true);
text_field_init(&email_field, 10, 50, 100, true);

// First field
InputResult result = text_field_get_focus(&name_field);
if (result == INPUT_RESULT_CLEAR) {
    // User canceled
    return;
}

// Second field
result = text_field_get_focus(&email_field);
if (result == INPUT_RESULT_ENTER) {
    // Process form data
    process_form(name_field.text, email_field.text);
}
```

## Implementation Guidelines

When implementing this architecture for the TI-84 CE:

1. **Blocking Function Design**
   - Design functions that handle a complete input cycle
   - Return meaningful results to the caller
   - Use parameters to pass information back up the call stack

2. **Layer Separation**
   - Each layer should only depend on the layer directly below it
   - Higher layers shouldn't need to know implementation details of lower layers

3. **State Management**
   - Keep state (like keyboard mode) in the appropriate layer
   - Use clean interfaces to expose state when needed

4. **Memory Efficiency**
   - Minimize dynamic memory allocation
   - Reuse buffers where possible

5. **CPU Efficiency**
   - Include small delays in polling loops to reduce CPU usage
   - Balance responsiveness with power efficiency

6. **User Feedback**
   - Provide visual feedback for keyboard modes
   - Ensure cursor blinks in text fields
   - Make state changes obvious to the user

This sequential architecture provides a practical approach to keyboard handling on the TI-84 CE platform, balancing the technical constraints with a clean design that separates concerns across layers.