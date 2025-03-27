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
#include "headers/ui_private.h"

/* ============================== State Variables ============================== */

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
void println_centered(const char* str) {
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
    println_centered("InputStr Testbed");
    draw_horizontal_line();
    os_SetCursorPos(2,0);
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


 /*
  *  __  __      _        _   _ ___   ___             _   _          
  * |  \/  |__ _(_)_ _   | | | |_ _| | __|  _ _ _  __| |_(_)___ _ _  
  * | |\/| / _` | | ' \  | |_| || |  | _| || | ' \/ _|  _| / _ \ ' \ 
  * |_|  |_\__,_|_|_||_|  \___/|___| |_| \_,_|_||_\__|\__|_\___/_||_|
  *                                                                 
  * The main function that manages the calculator's user interface.
  */

/**
 * Main UI function
 */
void run_something(void) {
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

/*
 *  _   _ _   _ _ _ _          ___             _   _             
 * | | | | |_(_) (_) |_ _  _  | __|  _ _ _  __| |_(_)___ _ _  ___
 * | |_| |  _| | | |  _| || | | _| || | ' \/ _|  _| / _ \ ' \(_-<
 *  \___/ \__|_|_|_|\__|\_, | |_| \_,_|_||_\__|\__|_\___/_||_/__/
 *                      |__/                                     
 * Miscellaneous utility functions for the calculator UI.
 */
