/**
 * MathSolver for TI-84 CE - Text-based UI Implementation
 * 
 * This file implements the text-based user interface for the MathSolver application
 * on the TI-84 CE calculator. It provides functions for rendering the UI, handling
 * user input, and managing the calculator's state.
 */

#include <tice.h>
#include <keypadc.h>
#include <string.h>
#include <stdio.h>
#include <ti/screen.h>
#include "headers/mathsolver.h"
#include "headers/ui.h"
#include "headers/ui_private.h"

/* ============================== State Variables ============================== */

/**
 * Enum representing the different states of the calculator.
 */
typedef enum {
    STATE_INPUT,    /**< Input state where the user enters expressions. */
    STATE_RESULT,   /**< Result state where the calculation result is displayed. */
    STATE_ERROR,    /**< Error state where error messages are shown. */
    STATE_HELP,     /**< Help state displaying usage instructions. */
    STATE_SETTINGS  /**< Settings state for configuring calculator options. */
} CalculatorState;

/** Current state of the calculator. */
static CalculatorState current_state = STATE_INPUT;

/** Buffer for storing the current expression entered by the user. */
static char current_expression[MAX_INPUT_LENGTH] = "";

/** Buffer for storing error messages. */
static char error_message[MAX_INPUT_LENGTH] = "";

/** Stores the result of the most recent calculation. */
static CalculationResult current_result;

/** Tracks the current scroll position for displaying calculation steps. */
static int step_scroll_position = 0;

/** Flag indicating whether to show detailed calculation steps. */
static bool show_step_details = false;

/** TI-84 CE screen dimensions. */
#define SCREEN_ROWS 8     /**< Number of rows on the screen (0-7). */
#define SCREEN_COLS 26    /**< Number of columns on the screen (0-25). */

/* ============================== Text UI Utilities ============================== */

/**
 * Clears the screen and resets the cursor to the home position.
 */
void clear_screen(void) {
    os_FontSelect(os_SmallFont);
    os_ClrHome();
    os_SetCursorPos(0, 0);
}

/**
 * Adds a blank line to the display.
 */
static void new_line(void) {
    os_NewLine();
}

/**
 * Prints a string to the screen.
 * 
 * @param str The string to print.
 */
void print(const char* str) {
    os_PutStrFull(str);
}

/**
 * Prints a string to the screen followed by a newline character.
 * 
 * @param str The string to print.
 */
void println(const char* str) {
    print(str);
    new_line();
}

/**
 * Centers a string on the screen.
 * 
 * @param text The string to center.
 */
void print_centered(const char* str) {
    int len = strlen(str);
    int padding = (SCREEN_COLS - len) / 2;
    print_format("%*s%s", padding, "", str);
}

/**
 * Prints a string truncated to fit within the specified maximum length.
 * 
 * @param text The string to print.
 * @param max_length The maximum length of the string to display.
 */
void print_truncated(const char* str, int max_length) {
    char buffer[SCREEN_COLS + 1];
    
    if (max_length > SCREEN_COLS) {
        max_length = SCREEN_COLS;
    }
    
    if ((int)strlen(str) <= max_length) {
        strncpy(buffer, str, max_length);
        buffer[max_length] = '\0';
        os_PutStrFull(buffer);
    } else {
        // Truncate with ellipsis
        strncpy(buffer, str, max_length - 3);
        buffer[max_length - 3] = '.';
        buffer[max_length - 2] = '.';
        buffer[max_length - 1] = '.';
        buffer[max_length] = '\0';
        os_PutStrFull(buffer);
    }
    new_line();
}

/**
 * Prints a formatted string to the screen.
 * 
 * @param format The format string to print.
 * @param ... Additional arguments to format.
 */
void print_format(const char* format, ...) {
    char buffer[255];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    println(buffer);
}

/**
 * Prints a formatted string centered on the screen.
 * 
 * @param format The format string to print.
 * @param ... Additional arguments to format.
 */
void print_format_centered(const char* format, ...) {
    char buffer[255];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    print_centered(buffer);
}

/**
 * Prints a formatted string truncated to fit within the specified maximum length.
 * 
 * @param max_length The maximum length of the string to display.
 * @param format The format string to print.
 * @param ... Additional arguments to format.
 */
void print_format_truncated(const char* format, ...) {
    char buffer[255];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    print_truncated(buffer, SCREEN_COLS - 1);
}

/**
 * Draws a horizontal line across the screen using the specified character.
 * 
 * @param line_char The character to use for the horizontal line.
 */
static void draw_horizontal_line(char line_char) {
    char line[SCREEN_COLS + 1];
    
    // Fill the line with the specified character
    for (int i = 0; i < SCREEN_COLS; i++) {
        line[i] = line_char;
    }
    line[SCREEN_COLS] = '\0';
    
    println(line);
}

/* ============================== UI Drawing Functions ============================== */

/**
 * Draws the header section of the calculator UI.
 */
void draw_header(void) {
    clear_screen();
    print_centered("MathSolver TI-84 CE");
    draw_horizontal_line('-');
    os_SetCursorPos(2,0);
}

/**
 * Displays the input prompt for entering expressions.
 */
void show_input_prompt(void) {
    draw_header();
    
    // Show the current arithmetic mode and precision
    ArithmeticType mode = get_arithmetic_mode();
    int precision       = get_precision();
    bool use_sig_digits = get_use_significant_digits();
    
    print_format("Mode: %s", 
                    mode == ARITHMETIC_NORMAL ? "Normal" : 
                    mode == ARITHMETIC_TRUNCATE ? "Truncate" : "Round");
    
    if (mode == ARITHMETIC_NORMAL)
        println("Prec: Default");
    else
        print_format("Prec: %d %s", 
                precision,
                use_sig_digits ? "sig" : "dec");

    new_line();
    println("Enter expression:");
}

/**
 * Displays the result of a calculation, including steps if available.
 * 
 * @param result Pointer to the CalculationResult structure containing the result data.
 */
void show_calculation_result(CalculationResult* result) {
    draw_header();
    
    // Show the expression and result
    print_format_truncated("Expr : %s", current_expression);
    print_format_truncated("Ans  : %s", result->formatted_result);
    print_format_truncated("Steps: %d", result->step_count);
    
    draw_horizontal_line('-');
    os_SetCursorPos(6, 0);
    
    // Only proceed with steps if we have any
    if (result->step_count > 1) {
        // Ensure step_scroll_position is within bounds
        if (step_scroll_position > result->step_count - 1) {
            step_scroll_position = 0;
        }
        if (step_scroll_position < 0) {
            step_scroll_position = result->step_count - 1;
        }
        
        // Display the current step
        if (step_scroll_position < result->step_count) {
            print_format_truncated("%d. %s = %s", 
                (step_scroll_position + 1), 
                result->steps[step_scroll_position].expression, 
                result->steps[step_scroll_position].result);
        }
    }
    
    // Navigation hints
    os_SetCursorPos(SCREEN_ROWS + 1, 0);
    print("<MODE>:Input <CLEAR>:Exit");
}

/**
 * Displays an error message on the screen.
 * 
 * @param message The error message to display.
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
 * Displays the help screen with usage instructions.
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
 * Displays the settings menu for configuring calculator options.
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
 * Gets an expression input from the user.
 * 
 * @param buffer Pointer to the buffer where the input will be stored.
 * @param buffer_size The size of the buffer.
 * @return True if input was provided, false if canceled.
 */
bool get_expression_input(char* buffer, int buffer_size) {
    // Prepare for input
    os_SetCursorPos(6, 0);
    
    // Make sure buffer is cleared
    buffer[0] = '\0';
    
    // Wait a moment to ensure screen is updated
    delay(100);
    
    // Get input - check documentation for exact parameters
    // The first parameter might need to be NULL
    os_GetStringInput("> ", buffer, buffer_size - 1);
    
    // Ensure null termination
    buffer[buffer_size - 1] = '\0';
    
    // Check if input was provided or canceled
    return (buffer[0] != '\0');
}

/**
 * Toggles the arithmetic mode between Normal, Truncate, and Round.
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
 * Adjusts the precision setting by the specified change amount.
 * 
 * @param change The amount to adjust the precision by.
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
 * Toggles between using significant digits and decimal places for precision.
 */
void toggle_significant_digits(void) {
    set_arithmetic_mode(get_arithmetic_mode(), get_precision(), !get_use_significant_digits());
}

/* ============================== Main UI Function ============================== */

void WaitForKeyDown(void) {
    while (!kb_AnyKey()) {
        kb_Scan();
        delay(50);
    }
}

void WaitForKeyRelease(void) {
    while (kb_AnyKey()) {
        kb_Scan();
        delay(50);
    }
}

void WaitForKeyPress(void) {
    WaitForKeyDown();
    WaitForKeyRelease();
}

/**
 * Main function that runs the calculator's user interface.
 * 
 * This function manages the main program loop, updates the display based on the
 * current state, and handles user input.
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
                
                while (true)
                {
                    // Handle key input for navigation
                    kb_Scan();
                    
                    if (kb_Data[6] & kb_Clear) {
                        // CLEAR key - exit
                        running = false;
                        break;
                    } else if (kb_Data[1] & kb_Mode) {
                        // MODE key - go back to input
                        current_state = STATE_INPUT;
                        break;
                    } else if (kb_Data[7] & kb_Up) {
                        // Up key - scroll steps up
                        if (step_scroll_position > 0) {
                            step_scroll_position--;
                        }
                        break;
                    } else if (kb_Data[7] & kb_Down) {
                        // Down key - scroll steps down
                        if (step_scroll_position < current_result.step_count - 1) {
                            step_scroll_position++;
                        }
                        break;
                    }
                }
                
                // Wait for key release
                delay(100);
                break;
                
            case STATE_ERROR:
                show_error_message(error_message);
                
                // Wait for any key press to return to input
                WaitForKeyDown();
                
                // Set next state
                current_state = STATE_INPUT;
                
                // Wait for key release
                WaitForKeyRelease();
                break;
                
            case STATE_HELP:
                show_help_screen();
                
                // Wait for any key press to return to input
                WaitForKeyDown();
                
                // Set next state
                current_state = STATE_INPUT;
                
                // Wait for key release
                WaitForKeyRelease();
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
