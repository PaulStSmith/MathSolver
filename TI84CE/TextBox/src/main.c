#include <tice.h>
#include <graphx.h>
#include <string.h>
#include <stdio.h>
#include <debug.h>
#include "headers/keyboard.h"
#include "headers/key_translator.h"
#include "headers/text_field.h"
#include "headers/gui.h"

// Simple form with two input fields: username and password
TextField username_field;
TextField password_field;

// Buffer to store results
char result_message[64];
bool show_result = false;

// Forward declarations
static void draw_kb_mode(KeyboardMode mode){

    int x = PADDING_X;
    int y = LCD_HEIGHT - GUI_LINE_HEIGHT - PADDING_Y;
    char mode_str[32];

    if ((mode & KB_MODE_2ND) == KB_MODE_2ND) {
        strcpy(mode_str, "\xe1");
    } else if ((mode & KB_MODE_ALPHA) == KB_MODE_ALPHA) {
        strcpy(mode_str, "\xe2");
    } else if ((mode & KB_MODE_LOWER) == KB_MODE_LOWER) {
        strcpy(mode_str, "\xe3");
    } else {
        strcpy(mode_str, " ");
    }

    GUI_print_text(x, y, mode_str);  // Print the mode string
    GUI_refresh();  // Refresh the screen to show the mode
}

// Callback when the form is submitted
void on_form_submit(TextField* field) {
    // Do something with the input data
    sprintf(result_message, "Login: %s (pwd len: %d)", 
            text_field_get_text(&username_field),
            strlen(text_field_get_text(&password_field)));
    show_result = true;
}

int main(void) {
    GUI_init();

    unsigned char a = '\x01';
    char buffer[256 + 15];
    memset(buffer, 0, sizeof(buffer));

    buffer[0] = ' ';
    for (unsigned int i = 1; i < sizeof(buffer); i++) {
        if (a % 16 == 0) {
            buffer[i++] = '\n';
            dbg_printf("%d - new line\n", i);
        }
        buffer[i] = a == '\x0a' ? ' ' : a;
        dbg_printf("%d - %d\n", i, a);
        a++;
    }
    buffer[sizeof(buffer) - 1] = '\0';

    GUI_print_text(PADDING_X, PADDING_Y, buffer);
    GUI_refresh();

    key_wait_any();

    GUI_end();
}

static int old_main(void) {
    // Initialize graphics and GUI (no need to call gfx_Begin explicitly)
    GUI_init();
    
    // Initialize text fields
    text_field_init(&username_field, 80, 60, 160, true);
    text_field_init(&password_field, 80, 100, 160, true);
    
    // Set password mode for the password field
    text_field_set_password_mode(&password_field, true, '*');
    
    // Link the fields together
    text_field_register_next(&username_field, &password_field);
    
    // Register callback for form submission
    text_field_on_enter(&password_field, on_form_submit);

    register_mode_change_callback(draw_kb_mode);
    
    // Display form
    bool running = true;
    TextResult result;
    
    while (running) {
        // Clear screen
        gfx_FillScreen(BG_COLOR);
        
        // Draw labels
        GUI_write_text(20, 62, "Username:");
        GUI_write_text(20, 102, "Password:");
        GUI_write_text_centered(30, "Login Form");
        
        // Draw instructions
        GUI_write_text(20, 140, "Press [ENTER] to submit");
        GUI_write_text(20, 160, "Press [CLEAR] to exit");
        
        // Draw the text fields
        text_field_draw(&username_field);
        text_field_draw(&password_field);
        
        // Display result if form was submitted
        if (show_result) {
            GUI_write_text(20, 180, result_message);
        }
        
        // Update screen using GUI_refresh
        GUI_refresh();
        
        // Process input for the username field
        if (!username_field.is_active && !password_field.is_active) {
            username_field.is_active = true;
            result = text_field_activate(&username_field);
            
            // Handle result
            if (result == TEXT_RESULT_CLEAR) {
                break;
            }
            else if (result == TEXT_RESULT_NEXT) {
                // Focus automatically moves to password_field
            }
        }
        
        // Process input for the password field if username field is done
        if (!username_field.is_active && !password_field.is_active) {
            password_field.is_active = true;
            result = text_field_activate(&password_field);
            
            // Handle result
            if (result == TEXT_RESULT_CLEAR) {
                break;
            }
            else if (result == TEXT_RESULT_ENTER) {
                // Form was submitted, result is displayed
            }
        }
        
        // Simple exit condition in addition to CLEAR key
        if (kb_Data[6] & kb_Clear) {
            running = false;
        }
        
        // Small delay to reduce CPU usage
        delay(10);
    }
    
    // Clean up
    unregister_mode_change_callback();
    text_field_free(&username_field);
    text_field_free(&password_field);
   
    // End graphics (GUI_end will call gfx_End)
    GUI_end();
    
    return 0;
}
