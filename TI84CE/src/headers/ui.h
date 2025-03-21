/**
 * MathSolver for TI-84 CE - Text-based UI Definitions
 * 
 * This header file defines constants and function prototypes for the 
 * text-based user interface of the MathSolver application. It provides 
 * functions for displaying UI components, handling user input, and 
 * managing settings.
 */

#ifndef UI_H
#define UI_H

#include "mathsolver.h"

/* ============================== UI Constants ============================== */

/** Maximum number of rows that can be displayed on the screen. */
#define MAX_DISPLAY_ROWS     10

/** Maximum number of columns that can be displayed on the screen. */
#define MAX_DISPLAY_COLS     26

/** Maximum number of items that can be displayed in a menu. */
#define MAX_MENU_ITEMS       5

/** Number of frames between cursor blinks for visual feedback. */
#define CURSOR_BLINK_RATE    10

/* ============================== Function Prototypes ============================== */

/**
 * Runs the main calculator user interface loop.
 */
void run_calculator_ui(void);

/* ------------------------------ UI Components ------------------------------ */

/**
 * Draws the header section of the UI.
 */
void draw_header(void);

/**
 * Displays the input prompt for the user to enter a mathematical expression.
 */
void show_input_prompt(void);

/**
 * Displays the result of a calculation.
 * 
 * @param result Pointer to the CalculationResult structure containing the result.
 */
void show_calculation_result(CalculationResult* result);

/**
 * Displays an error message on the screen.
 * 
 * @param message Pointer to the error message string.
 */
void show_error_message(const char* message);

/**
 * Displays the help screen with usage instructions.
 */
void show_help_screen(void);

/**
 * Displays the settings menu for user configuration.
 */
void show_settings_menu(void);

/* ------------------------------ Input Handling ------------------------------ */

/**
 * Prompts the user to input a mathematical expression.
 * 
 * @param buffer Pointer to the buffer where the input will be stored.
 * @param buffer_size Size of the buffer.
 * @return True if input was successfully received, false otherwise.
 */
bool get_expression_input(char* buffer, int buffer_size);

/**
 * Handles key inputs from the user.
 */
void handle_keys(void);

/* ------------------------------ Settings ------------------------------ */

/**
 * Toggles the arithmetic mode (e.g., basic or advanced).
 */
void toggle_arithmetic_mode(void);

/**
 * Adjusts the precision of calculations by a specified change.
 * 
 * @param change The amount to adjust the precision by.
 */
void adjust_precision(int change);

/**
 * Toggles the display of significant digits in results.
 */
void toggle_significant_digits(void);

#endif /* UI_H */
