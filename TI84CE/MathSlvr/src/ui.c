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
#include "headers/kb_handler.h"
#include "headers/mathsolver.h"
#include "headers/ui_private.h"

#define LOG_TAG "ui"
#include "headers/log.h"

/* ============================== State Variables ============================== */

/**
 * Enum representing the different states of the calculator.
 */
typedef enum {
    STATE_INPUT,    /**< Input state where the user enters expressions. */
    STATE_RESULT,   /**< Result state where the calculation result is displayed. */
    STATE_ERROR,    /**< Error state where error messages are shown. */
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

/** Flag indicating whether the calculator is running. */
static bool running = true;

/** Flag indicating whether the calculator is waiting for input. */
static bool waiting = false;

/** TI-84 CE screen dimensions. */
#define SCREEN_ROWS 9     /**< Number of rows on the screen (0-8). */
#define SCREEN_COLS 26    /**< Number of columns on the screen (0-25). */

/*
 *  _____        _     _   _ ___   _   _ _   _ _ _ _   _        
 * |_   _|____ _| |_  | | | |_ _| | | | | |_(_) (_) |_(_)___ ___
 *   | |/ -_) \ /  _| | |_| || |  | |_| |  _| | | |  _| / -_|_-<
 *   |_|\___/_\_\\__|  \___/|___|  \___/ \__|_|_|_|\__|_\___/__/
 *
 * Utility functions for rendering text and formatting output on the screen.
 */

/**
 * Clears the screen and resets the cursor to the home position.
 */
void clear_screen(void) {
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
 * Prints a string centered at the last row of the screen.
 * 
 * @param str The string to print.
 */
void print_footer(char* str) {
    os_SetCursorPos(SCREEN_ROWS, 0);
    print_format("%*s", SCREEN_COLS - 2, "");
    os_SetCursorPos(SCREEN_ROWS, 0);
    print_centered(str);
}

/**
 * Prints a string centered on the screen.
 * 
 * @param text The string to center.
 */
void print_centered(const char* str) {
    int len = strlen(str);
    int padL = (SCREEN_COLS - len) / 2;
    int padR = SCREEN_COLS - padL - len - 1;
    padL = (padL < 0) ? 0 : padL;
    padR = (padR < 0) ? 0 : padR;
    print_format("%*s%s%-*s", padL, "", str, padR, "");
}

/**
 * Prints a string centered on the screen, followed by a newline character.
 * 
 * @param str The string to center.
 */
void print_centeredln(const char* str) {
    int len = strlen(str);
    int padding = (SCREEN_COLS - len) / 2;
    println_format("%*s%s", padding, "", str);
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
    } else {
        // Truncate with ellipsis
        strncpy(buffer, str, max_length - 1);
        buffer[max_length - 1] = '\xCE'; // Ellipsis character
        buffer[max_length] = '\0';
    }
    println(buffer);
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
   
    print(buffer);
}

/**
 * Prints a formatted string to the screen, followed by a newline character.
 * 
 * @param format The format string to print.
 * @param ... Additional arguments to format.
 */
void println_format(const char* format, ...) {
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
void println_format_truncated(const char* format, ...) {
    char buffer[255];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    print_truncated(buffer, SCREEN_COLS - 1);
}

void print_right(const char* str) {
    unsigned int curRow;
    unsigned int curCol;
    os_GetCursorPos(&curRow, &curCol);
    os_SetCursorPos(curRow, SCREEN_COLS - strlen(str));
    print(str);
}

void println_right(const char* str) {
    print_right(str);
    new_line();
}

void print_format_right(const char* format, ...) {
    char buffer[255];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    print_right(buffer);
}

void println_format_right(const char* format, ...) {
    char buffer[255];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    println_right(buffer);
}

/**
 * Draws a horizontal line across the screen using the specified character.
 * 
 * @param line_char The character to use for the horizontal line.
 */
static void draw_horizontal_line(void) {

    unsigned int curRow;
    unsigned int curCol;
    char line[SCREEN_COLS + 1];

    os_GetCursorPos(&curRow, &curCol);
    os_SetCursorPos(curRow, 0);
    
    memset(line, '-', SCREEN_COLS); // Fill the buffer with dashes
    line[SCREEN_COLS] = '\0';      // Null-terminate the string
    print(line);
}

/*
 *  _   _ ___   ___                  _             ___             _   _             
 * | | | |_ _| |   \ _ _ __ ___ __ _(_)_ _  __ _  | __|  _ _ _  __| |_(_)___ _ _  ___
 * | |_| || |  | |) | '_/ _` \ V  V / | ' \/ _` | | _| || | ' \/ _|  _| / _ \ ' \(_-<
 *  \___/|___| |___/|_| \__,_|\_/\_/|_|_||_\__, | |_| \_,_|_||_\__|\__|_\___/_||_/__/
 *                                         |___/                                     
 * 
 * Functions for rendering different sections of the calculator UI.
 */

/**
 * Draws the header section of the calculator UI.
 */
void draw_header(void) {
    clear_screen();
    print_centered("MathSolver TI-84 CE");
    println("");
    draw_horizontal_line();
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
    
    println_format("Mode: %s", 
                    mode == ARITHMETIC_NORMAL ? "Normal" : 
                    mode == ARITHMETIC_TRUNCATE ? "Truncate" : "Round");
    
    if (mode == ARITHMETIC_NORMAL)
        println("Prec: Default");
    else
        println_format("Prec: %d %s", 
                precision,
                use_sig_digits ? "sig" : "dec");

    new_line();
    println("Enter expression:");
}

void show_calculation_result(CalculationResult* result) {
    int cnt = result->step_count;
    if (step_scroll_position < 0) {
        step_scroll_position = cnt;
    } else if (step_scroll_position > cnt)
    {
        step_scroll_position = 0;
    }
    
    clear_screen();
    os_SetCursorPos(0,0);
    print_format("%d/%d", step_scroll_position, cnt);
    print_right(get_mode_str());
        
    char diff_str[MAX_TOKEN_LENGTH];
    ArithmeticType mode = get_arithmetic_mode();
    if (mode != ARITHMETIC_NORMAL) {
        real_t left = result->normal_value;
        real_t right = result->value;
        real_t diff = os_RealSub(&left, &right);
        format_real(diff, diff_str);        
    }

    char rst[MAX_TOKEN_LENGTH];
    format_real(result->value, rst);
    
    if (step_scroll_position == 0) {
        os_SetCursorPos(2, 0);
        println("Formatado:");
        print_right(rst);
        
        if (mode != ARITHMETIC_NORMAL) {
            os_SetCursorPos(4, 0);
            char norm[MAX_TOKEN_LENGTH];
            format_real(result->normal_value, norm);
            println("Formatted:");
            print_right(norm);
            os_SetCursorPos(6, 0);
            println("Diff:");
            print_right(diff_str);
        }
    }
    else
    {
        CalculationStep* step = &result->steps[step_scroll_position - 1];
        os_SetCursorPos(1, 0);
        println_format_right("Oper: %s", step->operation);
        if(step->type == STEP_BINARY) {
            print("Left:");
            println_right(step->left_operand);
            print("Right:");
            println_right(step->right_operand);
        } else if(step->type == STEP_UNARY_LEFT) {
            print("Operand:");
            println_right(step->left_operand);
            new_line();
        } else if (step->type == STEP_UNARY_RIGHT)
        {
            print("Operand:");
            println_right(step->right_operand);
            new_line();
        }
        print("Result:");
        println_right(step->result);
    }

    print_footer("\xef\xf0:Scroll <MODE>:Settings");
}

/**
 * Displays an error message on the screen.
 * 
 * @param message The error message to display.
 */
void show_error_message(const char* message) {
    draw_header();
    println("ERROR:");
    println(message);
    print_footer("Press any key\xCE"); // \xCE - Ellipsis character
    kb_wait_any();
}

/**
 * Displays the settings menu for configuring calculator options.
 */
void show_settings_menu(void) {
    draw_header();
    print_centered("Settings");
    print_mode();
    print_precision();
    print_precision_type();
}

/**
 * Prints the current arithmetic mode.
 */
void print_mode(void){
    ArithmeticType mode = get_arithmetic_mode();

    os_SetCursorPos(4, 0);
    println_format("1. Mode: %s", 
        mode == ARITHMETIC_NORMAL ?   "Normal   " : 
        mode == ARITHMETIC_TRUNCATE ? "Truncate " : 
                                      "Round    ");
}

/**
 * Prints the current precision setting.
 */
void print_precision(void) {
    ArithmeticType mode = get_arithmetic_mode();
    int precision = get_precision();

    os_SetCursorPos(5, 0);
    if (mode == ARITHMETIC_NORMAL)
        println_format("2. Precision: Default");
    else
        println_format("2. Precision: %-*d  ", 8, precision);
}

/**
 * Prints the type of precision (significant digits or decimal places).
 */
void print_precision_type(void) {
    ArithmeticType mode = get_arithmetic_mode();
    bool use_sig_digits = get_use_significant_digits();

    os_SetCursorPos(6, 0);
    if (mode == ARITHMETIC_NORMAL)
        println_format("3. Type: %-*s", 10, "N/A");
    else
        println_format("3. Type: %s", use_sig_digits ? "Sig.Digits" : "Dec.Places");
}

/**
 * Displays a footer for selecting precision.
 */
void print_select_precision(void) {
    print_footer("0-9:Change");
}

/*
 *  ___                _     _  _              _ _ _           
 * |_ _|_ _  _ __ _  _| |_  | || |__ _ _ _  __| | (_)_ _  __ _ 
 *  | || ' \| '_ \ || |  _| | __ / _` | ' \/ _` | | | ' \/ _` |
 * |___|_||_| .__/\_,_|\__| |_||_\__,_|_||_\__,_|_|_|_||_\__, |
 *          |_|                                          |___/ 
 *
 * Functions for handling user input and managing key events.
 */

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

    print_mode();
    print_precision();
    print_precision_type();
}

/**
 * Adjusts the precision setting by the specified change amount.
 * 
 * @param change The amount to adjust the precision by.
 */
void adjust_precision(void) {
    ArithmeticType mode = get_arithmetic_mode();
    if (mode == ARITHMETIC_NORMAL) {
        return;
    }

    register_precision_kb();

    waiting = true;
    while (waiting) {
        kb_process();
        delay(50);
    }
}

/**
 * Toggles between using significant digits and decimal places for precision.
 */
void toggle_significant_digits(void) {
    set_arithmetic_mode(get_arithmetic_mode(), get_precision(), !get_use_significant_digits());
    print_precision_type();
}

/**
 * Sets the calculator state to input mode.
 */
void input_state(void) {
    current_state = STATE_INPUT;
}

/**
 * Sets the calculator state to settings mode.
 */
void settings_state(void) {
    current_state = STATE_SETTINGS;
}

/**
 * Exits the calculator UI.
 */
void leave(void) {
    waiting = false;
    running = false;
}

/*
 *  __  __      _        _   _ ___   ___             _   _          
 * |  \/  |__ _(_)_ _   | | | |_ _| | __|  _ _ _  __| |_(_)___ _ _  
 * | |\/| / _` | | ' \  | |_| || |  | _| || | ' \/ _|  _| / _ \ ' \ 
 * |_|  |_\__,_|_|_||_|  \___/|___| |_| \_,_|_||_\__|\__|_\___/_||_|
 *                                                                 
 * The main function that manages the calculator's user interface.
 */

/**
 * Main function that runs the calculator's user interface.
 * 
 * This function manages the main program loop, updates the display based on the
 * current state, and handles user input.
 */
void run_calculator_ui(void) {
    bool input_processed = false;
    
    // Initialize screen and keyboard
    os_ClrHome();
    kb_Reset();

    current_state = STATE_SETTINGS;
    
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
                            int len = strlen(current_expression);
                            for (int i = 0; i < len; i++) {
                                log_message("Char %d: '%c'  %2x", i, current_expression[i], current_expression[i]);
                            }
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
                        leave();
                    }
                } else {
                    // Reset flag after state change
                    input_processed = false;
                }
                break;
                
            case STATE_RESULT:
                register_result_kb();
                show_calculation_result(&current_result);

                while (running && current_state == STATE_RESULT) {
                    kb_process();
                    delay(50);
                }
                kb_clear();
                break;
                
            case STATE_ERROR:
                show_error_message(error_message);

                kb_wait_any();
                
                // Set next state
                current_state = STATE_INPUT;

                break;
                
            case STATE_SETTINGS:
                show_settings_menu();
                regiter_settings_kb();
                while (running && current_state == STATE_SETTINGS) {
                    kb_process();
                    delay(50);
                }
                kb_clear();
                break;
        }
    }
}

/*
 *  _  __         _                      _   ___          _    _            _   _          
 * | |/ /___ _  _| |__  ___  __ _ _ _ __| | | _ \___ __ _(_)__| |_ _ _ __ _| |_(_)___ _ _  
 * | ' </ -_) || | '_ \/ _ \/ _` | '_/ _` | |   / -_) _` | (_-<  _| '_/ _` |  _| / _ \ ' \ 
 * |_|\_\___|\_, |_.__/\___/\__,_|_| \__,_| |_|_\___\__, |_/__/\__|_| \__,_|\__|_\___/_||_|
 *           |__/                                   |___/                                  
 *
 * Functions for registering key events for different states.
 */

/**
 * Registers key events for the settings menu.
 */
static void regiter_settings_kb(void) {
    kb_clear();
    kb_register_press(KEY_1, toggle_arithmetic_mode);
    kb_register_press(KEY_2, adjust_precision);
    kb_register_press(KEY_3, toggle_significant_digits);
    kb_register_press(KEY_ENTER, input_state);
    kb_register_press(KEY_CLEAR, leave);
    waiting = false;

    print_footer("<ENT.>:Input <1-3>:Change");
}

/**
 * Converts a key to its corresponding number.
 * 
 * @param key The key to convert.
 * @return The number corresponding to the key, or -1 if not a number key.
 */
static int key_to_number(CombinedKey key) {
    if (key == KEY_0) return 0;
    if (key == KEY_1) return 1;
    if (key == KEY_2) return 2;
    if (key == KEY_3) return 3;
    if (key == KEY_4) return 4;
    if (key == KEY_5) return 5;
    if (key == KEY_6) return 6;
    if (key == KEY_7) return 7;
    if (key == KEY_8) return 8;
    if (key == KEY_9) return 9;
    return -1; // Not a number key
}

/**
 * Registers key events for adjusting precision.
 */
static void register_precision_kb(void) {
    kb_clear();
    kb_register_press(KEY_0, update_precision);
    kb_register_press(KEY_1, update_precision);
    kb_register_press(KEY_2, update_precision);
    kb_register_press(KEY_3, update_precision);
    kb_register_press(KEY_4, update_precision);
    kb_register_press(KEY_5, update_precision);
    kb_register_press(KEY_6, update_precision);
    kb_register_press(KEY_7, update_precision);
    kb_register_press(KEY_8, update_precision);
    kb_register_press(KEY_9, update_precision);
    kb_register_press(KEY_ENTER, regiter_settings_kb);
    print_footer("0-9:Change <ENTER>:back");
}

/**
 * Registers key events for the result display.
 */
static void register_result_kb(void) {
    kb_clear();
    kb_register_press(KEY_MODE, settings_state);
    kb_register_press(KEY_ENTER, input_state);
    kb_register_press(KEY_CLEAR, leave);
    kb_register_press(KEY_UP, scroll_up);
    kb_register_press(KEY_DOWN, scroll_down);
    print_footer("<ENT.>:Input <CLEAR>:exit");
}

/**
 * Scrolls up through calculation steps.
 */
static void scroll_up(void) {
    step_scroll_position--;
    show_calculation_result(&current_result);
}

/**
 * Scrolls down through calculation steps.
 */
static void scroll_down(void) {
    step_scroll_position++;
    show_calculation_result(&current_result);
}

/**
 * Updates the precision based on the last key pressed.
 */
static void update_precision(void) {
    CombinedKey key = kb_get_last_key();
    if (key) {
        int num = key_to_number(key);
        if (num >= 0) {
            set_precision(num);
            print_precision();
        }
    }
}

/*
 *  _   _ _   _ _ _ _          ___             _   _             
 * | | | | |_(_) (_) |_ _  _  | __|  _ _ _  __| |_(_)___ _ _  ___
 * | |_| |  _| | | |  _| || | | _| || | ' \/ _|  _| / _ \ ' \(_-<
 *  \___/ \__|_|_|_|\__|\_, | |_| \_,_|_||_\__|\__|_\___/_||_/__/
 *                      |__/                                     
 * Miscellaneous utility functions for the calculator UI.
 */

char* get_mode_str(void) {
    static char buffer[50];
    ArithmeticType mode = get_arithmetic_mode();

    sprintf(buffer, "%s", 
            mode == ARITHMETIC_NORMAL ? "Normal" : 
            mode == ARITHMETIC_TRUNCATE ? "Truncate" : "Round");

    if (mode != ARITHMETIC_NORMAL) {
        sprintf(buffer + strlen(buffer), " (%d %s)", 
                get_precision(), 
                get_use_significant_digits() ? "sig" : "dec");
    }

    return buffer;
}
