/**
 * MathSolver for TI-84 CE - Text-based UI Implementation
 * 
 * Implements calculator UI using ti/screen.h for text-based interface
 */

 #include <tice.h>
 #include <keypadc.h>
 #include <string.h>
 #include <stdio.h>
 #include <ti/screen.h>
 #include "headers/mathsolver.h"
 #include "headers/ui.h"
 
 /* ============================== State Variables ============================== */
 
 // Current state of the calculator
 typedef enum {
     STATE_INPUT,
     STATE_RESULT,
     STATE_ERROR,
     STATE_HELP,
     STATE_SETTINGS
 } CalculatorState;
 
 static CalculatorState current_state = STATE_INPUT;
 static char current_expression[MAX_INPUT_LENGTH] = "";
 static char error_message[MAX_INPUT_LENGTH] = "";
 static CalculationResult current_result;
 static int step_scroll_position = 0;
 static bool show_step_details = false;
 
 // TI-84 CE screen dimensions
 #define SCREEN_ROWS 8     // Number of rows (0-7)
 #define SCREEN_COLS 26    // Number of columns (0-25)
 
 /* ============================== Text UI Utilities ============================== */
 
 /**
  * Simple wrapper for os_ClrHome that also sets cursor to home
  */
 static void clear_screen(void) {
    os_FontSelect(os_SmallFont);
    os_ClrHome();
    os_SetCursorPos(0, 0);
 }
 
 /**
  * Adds a blank line to the display
  */
 static void new_line(void) {
     os_NewLine();
 }
 
 /**
  * Draws a horizontal line across the screen with the specified character
  */
 static void draw_horizontal_line(char line_char) {
     char line[SCREEN_COLS + 1];
     
     // Fill the line with the specified character
     for (int i = 0; i < SCREEN_COLS; i++) {
         line[i] = line_char;
     }
     line[SCREEN_COLS] = '\0';
     
     os_PutStrFull(line);
     new_line();
 }
 
 /**
  * Centers a string on the screen
  */
 static void print_centered(const char* text) {
     int text_len = strlen(text);
     if (text_len > SCREEN_COLS) {
         os_PutStrFull(text);
         new_line();
         return;
     }
     
     int padding = (SCREEN_COLS - text_len) / 2;
     
     // Create a padded string
     char padded[SCREEN_COLS + 1];
     
     // Fill with spaces
     memset(padded, ' ', SCREEN_COLS);
     padded[SCREEN_COLS] = '\0';
     
     // Copy the text to the center
     if (text_len > 0) {
         memcpy(padded + padding, text, text_len);
     }
     
     os_PutStrFull(padded);
     new_line();
 }
 
 /**
  * Truncates a string if it's too long for display
  */
 static void print_truncated(const char* text, int max_length) {
     char buffer[SCREEN_COLS + 1];
     
     if (max_length > SCREEN_COLS) {
         max_length = SCREEN_COLS;
     }
     
     if ((int)strlen(text) <= max_length) {
         strncpy(buffer, text, max_length);
         buffer[max_length] = '\0';
         os_PutStrFull(buffer);
     } else {
         // Truncate with ellipsis
         strncpy(buffer, text, max_length - 3);
         buffer[max_length - 3] = '.';
         buffer[max_length - 2] = '.';
         buffer[max_length - 1] = '.';
         buffer[max_length] = '\0';
         os_PutStrFull(buffer);
     }
 }
 
 /* ============================== UI Drawing Functions ============================== */
 
 /**
  * Draws the calculator header
  */
 void draw_header(void) {
     clear_screen();
     print_centered("MathSolver TI-84 CE");
     draw_horizontal_line('-');
 }
 
 /**
  * Shows the input prompt
  */
 void show_input_prompt(void) {
     draw_header();
     
     os_PutStrFull("Enter expression:");
     new_line();
     
     // Show the current arithmetic mode and precision
     char mode_str[SCREEN_COLS];
     char prec_str[SCREEN_COLS];
     ArithmeticType mode = get_arithmetic_mode();
     int precision = get_precision();
     bool use_sig_digits = get_use_significant_digits();
     
     sprintf(mode_str, "Mode: %s", 
             mode == ARITHMETIC_NORMAL ? "Normal" : 
             mode == ARITHMETIC_TRUNCATE ? "Truncate" : "Round");
    sprintf(prec_str, "Prec: %d %s", 
            precision,
            use_sig_digits ? "sig" : "dec");
     
     os_PutStrFull(mode_str);
     new_line();
     os_PutStrFull(prec_str);
 }
 
 /**
  * Shows the calculation result
  */
 void show_calculation_result(CalculationResult* result) {
     draw_header();
     
     // Show the expression and result
     os_PutStrFull("Expr: ");
     print_truncated(current_expression, SCREEN_COLS - 6);
     new_line();
     
     os_PutStrFull("Ans: ");
     // Ensure the result is properly formatted
     char result_buffer[32];
     strncpy(result_buffer, result->formatted_result, 31);
     result_buffer[31] = '\0';
     os_PutStrFull(result_buffer);
     new_line();
     
     draw_horizontal_line('-');
     
     // Only proceed with steps if we have any
     if (result->step_count > 0) {
         // Show calculation steps heading
         char step_heading[SCREEN_COLS + 1];
         sprintf(step_heading, "Steps (%d):", result->step_count);
         step_heading[SCREEN_COLS] = '\0';
         os_PutStrFull(step_heading);
         new_line();
         
         // Calculate how many steps we can display
         int visible_steps = 1; // Just show one at a time to keep it simple
         
         // Ensure step_scroll_position is within bounds
         if (step_scroll_position > result->step_count - 1) {
             step_scroll_position = result->step_count - 1;
         }
         if (step_scroll_position < 0) {
             step_scroll_position = 0;
         }
         
         // Display the current step
         if (step_scroll_position < result->step_count) {
             char step_buffer[16];
             sprintf(step_buffer, "%d. ", step_scroll_position + 1);
             step_buffer[15] = '\0';
             os_PutStrFull(step_buffer);
             
             // Show a simplified step
             char step_text[SCREEN_COLS + 1];
             strncpy(step_text, result->steps[step_scroll_position].expression, SCREEN_COLS - 10);
             step_text[SCREEN_COLS - 10] = '\0';
             os_PutStrFull(step_text);
             new_line();
         }
     }
     
     // Navigation hints
     new_line();
     os_PutStrFull("[MODE]:Input [CLEAR]:Exit");
 }
 
 /**
  * Shows an error message
  */
 void show_error_message(const char* message) {
     draw_header();
     
     os_PutStrFull("ERROR:");
     new_line();
     
     // Print the error message, limited to one line for simplicity
     print_truncated(message, SCREEN_COLS);
     
     new_line();
     new_line();
     
     os_PutStrFull("Press any key to continue");
 }
 
 /**
  * Shows the help screen
  */
 void show_help_screen(void) {
     draw_header();
     
     print_centered("Help");
     new_line();
     
     os_PutStrFull("Operators: +,-,*,/,^,!");
     new_line();
     
     os_PutStrFull("Functions: sin,cos,tan");
     new_line();
     
     os_PutStrFull("Constants: pi,e,phi");
     new_line();
     
     new_line();
     os_PutStrFull("Press any key to return");
 }
 
 /**
  * Shows the settings menu
  */
 void show_settings_menu(void) {
     ArithmeticType mode = get_arithmetic_mode();
     int precision = get_precision();
     bool use_sig_digits = get_use_significant_digits();
     
     draw_header();
     
     print_centered("Settings");
     new_line();
     
     // Display current settings
     os_PutStrFull("1.Mode: ");
     os_PutStrFull(mode == ARITHMETIC_NORMAL ? "Normal" : 
                 mode == ARITHMETIC_TRUNCATE ? "Truncate" : "Round");
     new_line();
     
     os_PutStrFull("2.Precision: ");
     char prec_str[8];
     sprintf(prec_str, "%d", precision);
     os_PutStrFull(prec_str);
     new_line();
     
     os_PutStrFull("3.Type: ");
     os_PutStrFull(use_sig_digits ? "Sig.Digits" : "Dec.Places");
     new_line();
     
     new_line();
     os_PutStrFull("Press # to change setting");
 }
 
 /* ============================== Input Handling ============================== */
 
 /**
  * Gets expression input from user
  */
 bool get_expression_input(char* buffer, int buffer_size) {
     // Prepare for input
     os_SetCursorPos(3, 0);
     os_PutStrFull("> ");
     
     // Make sure buffer is cleared
     buffer[0] = '\0';
     
     // Wait a moment to ensure screen is updated
     delay(100);
     
     // Get input - check documentation for exact parameters
     // The first parameter might need to be NULL
     os_GetStringInput(NULL, buffer, buffer_size - 1);
     
     // Ensure null termination
     buffer[buffer_size - 1] = '\0';
     
     // Check if input was provided or canceled
     return (buffer[0] != '\0');
 }
 
 /**
  * Updates settings when user changes arithmetic mode
  */
 void toggle_arithmetic_mode(void) {
     ArithmeticType mode = get_arithmetic_mode();
     
     // Cycle through available modes
     switch (mode) {
         case ARITHMETIC_NORMAL:
             set_arithmetic_mode(ARITHMETIC_TRUNCATE, get_precision(), get_use_significant_digits());
             break;
         case ARITHMETIC_TRUNCATE:
             set_arithmetic_mode(ARITHMETIC_ROUND, get_precision(), get_use_significant_digits());
             break;
         case ARITHMETIC_ROUND:
             set_arithmetic_mode(ARITHMETIC_NORMAL, get_precision(), get_use_significant_digits());
             break;
     }
 }
 
 /**
  * Updates settings when user changes precision
  */
 void adjust_precision(int change) {
     int precision = get_precision();
     precision += change;
     
     // Ensure precision is within reasonable bounds
     if (precision < 0) precision = 0;
     if (precision > 10) precision = 10;
     
     set_arithmetic_mode(get_arithmetic_mode(), precision, get_use_significant_digits());
 }
 
 /**
  * Updates settings when user toggles between decimal places and significant digits
  */
 void toggle_significant_digits(void) {
     set_arithmetic_mode(get_arithmetic_mode(), get_precision(), !get_use_significant_digits());
 }
 
 /* ============================== Main UI Function ============================== */
 
 /**
  * Main UI function that runs the calculator interface
  */
 void run_calculator_ui(void) {
     bool running = true;
     bool input_processed = false;
     
     // Initialize screen and keyboard
     os_ClrHome();
     kb_Reset();
     
     // Main program loop
     while (running) {
         // Update the display based on current state
         switch (current_state) {
             case STATE_INPUT:
                 // Only show prompt if we haven't just processed input
                 if (!input_processed) {
                     show_input_prompt();
                     
                     // Get input from user
                     if (get_expression_input(current_expression, MAX_INPUT_LENGTH)) {
                         // User entered an expression
                         if (strlen(current_expression) > 0) {
                             // Set flag to prevent redisplaying input prompt
                             input_processed = true;
                             
                             // Evaluate the expression
                             if (evaluate_expression_string(current_expression, &current_result)) {
                                 current_state = STATE_RESULT;
                                 step_scroll_position = 0;
                                 show_step_details = false;
                             } else {
                                 // Set error message
                                 strcpy(error_message, "Invalid expression");
                                 current_state = STATE_ERROR;
                             }
                         }
                     } else {
                         // User cancelled input
                         running = false;
                     }
                 } else {
                     // Reset flag after state change
                     input_processed = false;
                 }
                 break;
                 
             case STATE_RESULT:
                 show_calculation_result(&current_result);
                 
                 // Handle key input for navigation
                 kb_Scan();
                 
                 if (kb_Data[6] & kb_Clear) {
                     // CLEAR key - exit
                     running = false;
                 } else if (kb_Data[1] & kb_Mode) {
                     // MODE key - go back to input
                     current_state = STATE_INPUT;
                 } else if (kb_Data[3] & kb_Up) {
                     // Up key - scroll steps up
                     if (step_scroll_position > 0) {
                         step_scroll_position--;
                     }
                 } else if (kb_Data[3] & kb_Down) {
                     // Down key - scroll steps down
                     if (step_scroll_position < current_result.step_count - 1) {
                         step_scroll_position++;
                     }
                 }
                 
                 // Wait for key release
                 delay(100);
                 break;
                 
             case STATE_ERROR:
                 show_error_message(error_message);
                 
                 // Wait for any key press to return to input
                 while (!kb_AnyKey()) {
                     kb_Scan();
                     delay(50);
                 }
                 
                 // Set next state
                 current_state = STATE_INPUT;
                 
                 // Wait for key release
                 while (kb_AnyKey()) {
                     kb_Scan();
                     delay(50);
                 }
                 break;
                 
             case STATE_HELP:
                 show_help_screen();
                 
                 // Wait for any key press to return to input
                 while (!kb_AnyKey()) {
                     kb_Scan();
                     delay(50);
                 }
                 
                 // Set next state
                 current_state = STATE_INPUT;
                 
                 // Wait for key release
                 while (kb_AnyKey()) {
                     kb_Scan();
                     delay(50);
                 }
                 break;
                 
             case STATE_SETTINGS:
                 show_settings_menu();
                 
                 // Handle settings changes
                 kb_Scan();
                 
                 if (kb_Data[1] & kb_Mode) {
                     // MODE key - return to input
                     current_state = STATE_INPUT;
                 } else if (kb_Data[4] & kb_1) {
                     // 1 key - change arithmetic mode
                     toggle_arithmetic_mode();
                 } else if (kb_Data[4] & kb_2) {
                     // 2 key - adjust precision
                     adjust_precision(1);  // Just increment for simplicity
                 } else if (kb_Data[4] & kb_3) {
                     // 3 key - toggle precision type
                     toggle_significant_digits();
                 }
                 
                 // Wait for key release
                 delay(100);
                 break;
         }
         
         // Check for global key combinations
         kb_Scan();
         
         // Help and Settings access through ALPHA combinations
         if (kb_Data[2] & kb_Alpha) {
             // Check for ALPHA + 1 (Help)
             if (kb_Data[4] & kb_1) {
                 current_state = STATE_HELP;
             }
             // Check for ALPHA + 2 (Settings)
             else if (kb_Data[4] & kb_2) {
                 current_state = STATE_SETTINGS;
             }
             
             // Wait for key release
             while (kb_AnyKey()) {
                 kb_Scan();
                 delay(50);
             }
         }
         
         // Wait a bit to prevent flicker
         delay(50);
     }
 }