#include <tice.h>
#include <graphx.h>
#include <string.h>
#include <stdio.h>
#include "headers/gui_private.h"
#include "headers/gui.h"
#include "headers/kb_handler.h"

// Global variables
HScrollTextField* active_field = NULL;

// Function prototypes
void setup_keyboard_callbacks(void);
void handle_left_press(void);
void handle_right_press(void);
void handle_del_press(void);
void handle_number_press(void);
void handle_add_press(void);
void handle_sub_press(void);
void handle_mul_press(void);
void handle_div_press(void);
void handle_lparen_press(void);
void handle_rparen_press(void);
void handle_sin_press(void);
void handle_cos_press(void);
void handle_tan_press(void);
void handle_key_hold(int hold_time);

int main(void) {
    // Initialize the GUI system
    GUI_init();
    
    // Initialize keyboard handler
    kb_init();
    setup_keyboard_callbacks();
    
    // Create and initialize a scrollable text field
    HScrollTextField input_field;
    GUI_hscroll_init(&input_field, 10, 50, LCD_WIDTH - 20, true);
    input_field.is_active = true;
    GUI_hscroll_set_text(&input_field, "sin(x) + 5");
    
    // Set as the active field (for keyboard input)
    active_field = &input_field;
    
    // Main loop
    bool running = true;
    
    while (running) {
        // Process keyboard events
        kb_process();
        
        // Check if user pressed clear to exit
        if (kb_is_key_pressed(MAKE_KEY(6, kb_Clear))) {
            if (input_field.text_length > 0) {
                GUI_hscroll_clear(&input_field);
            } else {
                // Exit if the field is empty
                running = false;
                continue;
            }
        }

        // Clear the screen
        gfx_FillScreen(BG_COLOR);
        
        // Draw a title
        GUI_write_text_centered(10, "Math Expression Editor");
        
        // Draw instructions
        GUI_write_text(10, 30, "Use arrow keys to move cursor");
        
        // Draw the scrollable text field
        GUI_hscroll_draw(&input_field);
        
        // Display current expression length
        char length_info[32];
        sprintf(length_info, "Length: %d chars", input_field.text_length);
        GUI_write_text(10, 80, length_info);
        
        // Preview section
        GUI_write_text(10, 110, "Expression:");
        GUI_print_text(10, 130, input_field.text);
        
        // Render all changes to the screen
        gfx_BlitBuffer();
        
        // Small delay
        delay(50);
    }
    
    // Clean up
    kb_clear(); // Clear all keyboard callbacks
    GUI_hscroll_free(&input_field);
    GUI_end();
    
    return 0;
}

// Set up keyboard callbacks
void setup_keyboard_callbacks(void) {
    // Arrow keys for cursor movement
    kb_register_press(MAKE_KEY(7, kb_Left), handle_left_press);
    kb_register_press(MAKE_KEY(7, kb_Right), handle_right_press);
    
    // Register hold callbacks for repeat movement
    kb_register_hold(MAKE_KEY(7, kb_Left), handle_key_hold, 300, true, 100);
    kb_register_hold(MAKE_KEY(7, kb_Right), handle_key_hold, 300, true, 100);
    
    // Delete key
    kb_register_press(MAKE_KEY(1, kb_Del), handle_del_press);
    
    // Number keys (just register one for demo)
    kb_register_press(MAKE_KEY(3, kb_1), handle_number_press);
    kb_register_press(MAKE_KEY(4, kb_2), handle_number_press);
    kb_register_press(MAKE_KEY(5, kb_3), handle_number_press);
    kb_register_press(MAKE_KEY(3, kb_4), handle_number_press);
    kb_register_press(MAKE_KEY(4, kb_5), handle_number_press);
    kb_register_press(MAKE_KEY(5, kb_6), handle_number_press);
    kb_register_press(MAKE_KEY(3, kb_7), handle_number_press);
    kb_register_press(MAKE_KEY(4, kb_8), handle_number_press);
    kb_register_press(MAKE_KEY(5, kb_9), handle_number_press);
    kb_register_press(MAKE_KEY(3, kb_0), handle_number_press);
    
    // Basic operators
    kb_register_press(MAKE_KEY(6, kb_Add), handle_add_press);
    kb_register_press(MAKE_KEY(6, kb_Sub), handle_sub_press);
    kb_register_press(MAKE_KEY(6, kb_Mul), handle_mul_press);
    kb_register_press(MAKE_KEY(6, kb_Div), handle_div_press);
    
    // Parentheses
    kb_register_press(MAKE_KEY(4, kb_LParen), handle_lparen_press);
    kb_register_press(MAKE_KEY(5, kb_RParen), handle_rparen_press);
    
    // Functions
    kb_register_press(MAKE_KEY(3, kb_Sin), handle_sin_press);
    kb_register_press(MAKE_KEY(4, kb_Cos), handle_cos_press);
    kb_register_press(MAKE_KEY(5, kb_Tan), handle_tan_press);
}

// Handle left arrow press
void handle_left_press(void) {
    if (active_field) {
        GUI_hscroll_cursor_left(active_field);
    }
}

// Handle right arrow press
void handle_right_press(void) {
    if (active_field) {
        GUI_hscroll_cursor_right(active_field);
    }
}

// Handle delete key press
void handle_del_press(void) {
    if (active_field) {
        // If cursor is at the end, act like backspace
        if (active_field->cursor_position == active_field->text_length) {
            GUI_hscroll_backspace_char(active_field);
        } else {
            // Otherwise delete the character at cursor position
            GUI_hscroll_delete_char(active_field);
        }
    }
}

// Handle number key press - determine which number was pressed
void handle_number_press(void) {
    if (!active_field) return;
    
    CombinedKey key = kb_get_last_key();
    char digit = '0';
    
    // Determine which digit was pressed based on the key
    if (key == MAKE_KEY(3, kb_0)) digit = '0';
    else if (key == MAKE_KEY(3, kb_1)) digit = '1';
    else if (key == MAKE_KEY(4, kb_2)) digit = '2';
    else if (key == MAKE_KEY(5, kb_3)) digit = '3';
    else if (key == MAKE_KEY(3, kb_4)) digit = '4';
    else if (key == MAKE_KEY(4, kb_5)) digit = '5';
    else if (key == MAKE_KEY(5, kb_6)) digit = '6';
    else if (key == MAKE_KEY(3, kb_7)) digit = '7';
    else if (key == MAKE_KEY(4, kb_8)) digit = '8';
    else if (key == MAKE_KEY(5, kb_9)) digit = '9';
    
    GUI_hscroll_insert_char(active_field, digit);
}

// Handle addition key
void handle_add_press(void) {
    if (active_field) {
        GUI_hscroll_insert_char(active_field, '+');
    }
}

// Handle subtraction key
void handle_sub_press(void) {
    if (active_field) {
        GUI_hscroll_insert_char(active_field, '-');
    }
}

// Handle multiplication key
void handle_mul_press(void) {
    if (active_field) {
        GUI_hscroll_insert_char(active_field, '*');
    }
}

// Handle division key
void handle_div_press(void) {
    if (active_field) {
        GUI_hscroll_insert_char(active_field, '/');
    }
}

// Handle left parenthesis
void handle_lparen_press(void) {
    if (active_field) {
        GUI_hscroll_insert_char(active_field, '(');
    }
}

// Handle right parenthesis
void handle_rparen_press(void) {
    if (active_field) {
        GUI_hscroll_insert_char(active_field, ')');
    }
}

// Handle sine function
void handle_sin_press(void) {
    if (active_field) {
        GUI_hscroll_append(active_field, "sin(");
    }
}

// Handle cosine function
void handle_cos_press(void) {
    if (active_field) {
        GUI_hscroll_append(active_field, "cos(");
    }
}

// Handle tangent function
void handle_tan_press(void) {
    if (active_field) {
        GUI_hscroll_append(active_field, "tan(");
    }
}

// Handle key hold (for repeating movements)
void handle_key_hold(int hold_time) {
    if (!active_field) return;
    
    CombinedKey key = kb_get_last_key();
    
    if (key == MAKE_KEY(7, kb_Left)) {
        GUI_hscroll_cursor_left(active_field);
    } else if (key == MAKE_KEY(7, kb_Right)) {
        GUI_hscroll_cursor_right(active_field);
    }
}