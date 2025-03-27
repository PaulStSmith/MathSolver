/**
 * MathSolver for TI-84 CE - Main Program
 * 
 * Main entry point for the calculator application.
 * This program initializes the math solver, sets up default variables,
 * configures arithmetic settings, and runs the calculator UI.
 */

#include <tice.h>
#include <graphx.h>
#include <ti/screen.h>
#include <keypadc.h>
#include <string.h>
#include <stdio.h>
#include "headers/ui.h"
#include "headers/log.h"
// #include "headers/main_private.h"
#include "headers/kb_handler.h"
#include "headers/kb_mapping.h"

// Size and position constants for text display
#define TEXT_X 10
#define TEXT_Y 40
#define BUFFER_SIZE 50
#define MAX_INPUT_LEN 30

// Colors for the graphical interface
#define BG_COLOR 0xFF
#define TEXT_COLOR 0x00
#define HIGHLIGHT_COLOR 0x10

// Example input buffer implementation
char input_buffer[MAX_INPUT_LEN + 1] = {0};
int cursor_position = 0;

// Function prototypes for this example
void draw_status(const char* status_text);
void draw_input(void);
void add_char_to_input(char c);
void handle_special_key(int key_value);

int main(void) {
    // Initialize graphics
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_FillScreen(BG_COLOR);
    gfx_SetTextFGColor(TEXT_COLOR);
    gfx_SetTextBGColor(BG_COLOR);
    
    // Initialize keyboard handler
    kb_init();
    KeyboardState kb_state = key_mapping_init();
    
    // Initial draw
    gfx_PrintStringXY("TI-84 CE Input Example", TEXT_X, TEXT_Y - 20);
    draw_status("Normal Mode");
    draw_input();
    gfx_BlitBuffer();
    
    // Main loop
    bool running = true;
    while (running) {
        // Process keyboard events
        kb_process();
        
        // Check for keys
        if (kb_AnyKey()) {
            // Find which key was pressed
            CombinedKey pressed_key = kb_get_last_key();
            
            // Check if it's a mode key (Alpha or 2nd)
            if (key_mapping_process_mode_keys(pressed_key, &kb_state)) {
                // Update mode display
                switch (kb_state.current_mode) {
                    case KB_MODE_NORMAL:
                        draw_status("Normal Mode");
                        break;
                    case KB_MODE_ALPHA:
                        draw_status("Alpha Mode (Temporary)");
                        break;
                    case KB_MODE_2ND:
                        draw_status("2nd Mode");
                        break;
                    case KB_MODE_ALPHA_LOCK:
                        draw_status("Alpha Lock Mode (A-Lock)");
                        break;
                }
            } else {
                // Get the value for the current key in the current mode
                int key_value = key_mapping_get_value(pressed_key, kb_state);
                
                // Debug: show key value
                char key_str[16];
                key_mapping_value_to_string(key_value, key_str);
                gfx_PrintStringXY("Key: ", TEXT_X, TEXT_Y + 60);
                gfx_PrintStringXY(key_str, TEXT_X + 40, TEXT_Y + 60);
                
                // Handle the key based on its value
                if (key_value >= 32 && key_value < 128) {
                    // Regular character
                    add_char_to_input((char)key_value);
                } else {
                    // Special key (function, control, etc.)
                    handle_special_key(key_value);
                }
                
                // Check for exit condition (Clear key)
                if (key_value == KB_KEY_CLEAR && input_buffer[0] == '\0') {
                    // Exit the program if Clear is pressed and input is empty
                    running = false;
                }
            }
            
            // Redraw the input
            draw_input();
            gfx_BlitBuffer();
            
            // Wait for key release to avoid repeats
            debounce();
        }
        
        // Small delay to reduce CPU usage
        delay(10);
    }
    
    // Cleanup
    gfx_End();
    return 0;
}

/**
 * Draws the current status text at the top of the screen
 */
void draw_status(const char* status_text) {
    // Clear status area
    gfx_SetColor(BG_COLOR);
    gfx_FillRectangle(0, 0, 320, 20);
    
    // Draw status text
    gfx_SetTextXY(TEXT_X, 5);
    gfx_PrintString(status_text);
}

/**
 * Draws the current input buffer with cursor
 */
void draw_input(void) {
    // Clear input area
    gfx_SetColor(BG_COLOR);
    gfx_FillRectangle(0, TEXT_Y, 320, 30);
    
    // Draw input box
    gfx_SetColor(TEXT_COLOR);
    gfx_Rectangle(TEXT_X, TEXT_Y, 300, 20);
    
    // Draw input text and cursor
    gfx_PrintStringXY(input_buffer, TEXT_X + 5, TEXT_Y + 5);
    
    // Draw cursor position
    char draw_str[cursor_position + 1];
    snprintf(draw_str, sizeof(draw_str), "%s", input_buffer);
    int cursor_x = 4 + TEXT_X + gfx_GetStringWidth(draw_str);
    gfx_SetColor(HIGHLIGHT_COLOR);
    gfx_Line(cursor_x, TEXT_Y + 3, cursor_x, TEXT_Y + 15);
}

/**
 * Adds a character to the input buffer at current cursor position
 */
void add_char_to_input(char c) {
    // Check if we have room
    if (strlen(input_buffer) < MAX_INPUT_LEN) {
        // Make room for the new character
        memmove(input_buffer + cursor_position + 1, 
                input_buffer + cursor_position,
                strlen(input_buffer) - cursor_position + 1);
        
        // Insert the character
        input_buffer[cursor_position] = c;
        
        // Advance cursor
        cursor_position++;
    }
}

/**
 * Handle special keys like backspace, arrows, etc.
 */
void handle_special_key(int key_value) {
    switch (key_value) {
        case KB_KEY_LEFT:
            // Move cursor left
            if (cursor_position > 0) {
                cursor_position--;
            }
            break;
            
        case KB_KEY_RIGHT:
            // Move cursor right
            if (cursor_position < (int)strlen(input_buffer)) {
                cursor_position++;
            }
            break;
            
        case KB_KEY_DEL:
            // Delete character at cursor
            if (cursor_position < (int)strlen(input_buffer)) {
                memmove(input_buffer + cursor_position,
                        input_buffer + cursor_position + 1,
                        strlen(input_buffer) - cursor_position);
            }
            break;
            
        case KB_KEY_CLEAR:
            // Clear entire input
            input_buffer[0] = '\0';
            cursor_position = 0;
            break;
            
        case KB_KEY_ENTER:
            // Process the input (for demonstration, just clear)
            gfx_PrintStringXY("Input: ", TEXT_X, TEXT_Y + 30);
            gfx_PrintString(input_buffer);
            
            // Clear the input after processing
            input_buffer[0] = '\0';
            cursor_position = 0;
            break;
            
        default: {
            // For function keys, just show what was pressed
            char func_name[16];
            key_mapping_value_to_string(key_value, func_name);
            gfx_PrintStringXY("Func: ", TEXT_X, TEXT_Y + 80);
            gfx_PrintStringXY(func_name, TEXT_X + 40, TEXT_Y + 80);
            break;
        }
    }
}
