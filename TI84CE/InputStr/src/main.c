#include <tice.h>
#include <graphx.h>
#include <string.h>
#include <stdio.h>
#include "headers/gui.h"
#include "headers/kb_handler.h"
#include "headers/input_field.h"

// Custom mode indicator callback
void draw_mode_indicator(KeyboardMode mode, int x, int y) {
    char* mode_texts[] = {
        "NORMAL",    // KB_MODE_NORMAL
        "2ND",       // KB_MODE_2ND
        "ALPHA",     // KB_MODE_ALPHA
        "A-LOCK",    // KB_MODE_ALPHA_LOCK
        "a-lock"     // KB_MODE_ALPHA_LOWER_LOCK
    };
    
    if (mode == KB_MODE_NORMAL) return;  // Don't display for normal mode
    
    // Draw a background box
    gfx_SetColor(0x10);  // Light blue
    gfx_FillRectangle(x + 2, y, 50, 10);
    
    // Draw the mode text
    gfx_SetTextFGColor(0);  // Black
    gfx_SetTextBGColor(0x10);  // Light blue
    gfx_PrintStringXY(mode_texts[mode], x + 4, y + 1);
    
    // Reset colors
    gfx_SetTextFGColor(0);
    gfx_SetTextBGColor(255);
}

int main(void) {
    // Initialize the GUI system
    GUI_init();
    
    // Initialize keyboard handler
    kb_init();
    
    // Create input fields
    InputField name_field, equation_field;
    
    // Initialize the fields
    input_field_init(&name_field, 10, 30, LCD_WIDTH - 20, true);
    input_field_init(&equation_field, 10, 80, LCD_WIDTH - 20, true);
    
    // Register custom mode indicator callback
    input_field_register_mode_indicator(&name_field, draw_mode_indicator);
    input_field_register_mode_indicator(&equation_field, draw_mode_indicator);
    
    // Chain fields together
    input_field_register_next(&name_field, &equation_field);
    
    // Set initial text
    input_field_set_text(&name_field, "");
    input_field_set_text(&equation_field, "sin(x) + 5");
    
    // Main loop
    bool running = true;
    InputResult result;
    
    // Draw the app's UI frame
    gfx_FillScreen(255);  // White background
    GUI_write_text(10, 10, "Math Expression Editor");
    GUI_write_text(10, 20, "Enter your name:");
    GUI_write_text(10, 70, "Enter your equation:");
    GUI_write_text(10, 150, "Press Enter to submit, Clear to exit");
    gfx_BlitBuffer();
    
    // Process input for the first field
    result = input_field_get_focus(&name_field);
    
    if (result == INPUT_RESULT_NEXT) {
        // User pressed Enter, move to equation field
        result = input_field_get_focus(&equation_field);
    }
    
    // Process final result
    switch (result) {
        case INPUT_RESULT_ENTER:
            // User pressed Enter on the equation field
            gfx_FillScreen(255);
            GUI_write_text(10, 10, "Results:");
            
            // Show the entered data
            char name_buffer[64];
            sprintf(name_buffer, "Name: %s", input_field_get_text(&name_field));
            GUI_write_text(10, 30, name_buffer);
            
            char equation_buffer[64];
            sprintf(equation_buffer, "Equation: %s", input_field_get_text(&equation_field));
            GUI_write_text(10, 50, equation_buffer);
            
            GUI_write_text(10, 80, "Press any key to exit");
            gfx_BlitBuffer();
            
            // Wait for a key press before exiting
            kb_wait_any();
            break;
            
        case INPUT_RESULT_CLEAR:
            // User pressed Clear on an empty field
            gfx_FillScreen(255);
            GUI_write_text_centered(LCD_HEIGHT/2, "Operation canceled");
            gfx_BlitBuffer();
            delay(1000);
            break;
            
        default:
            // Other exit condition
            break;
    }
    
    // Clean up
    input_field_free(&name_field);
    input_field_free(&equation_field);
    kb_clear();
    GUI_end();
    
    return 0;
}
