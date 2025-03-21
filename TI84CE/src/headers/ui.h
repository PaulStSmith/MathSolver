/**
 * MathSolver for TI-84 CE - Text-based UI Definitions
 * 
 * User interface functions for the math solver using ti/screen.h
 */

 #ifndef UI_H
 #define UI_H
 
 #include "mathsolver.h"
 
 /* ============================== UI Constants ============================== */
 
 #define MAX_DISPLAY_ROWS     10    // Maximum number of rows to display
 #define MAX_DISPLAY_COLS     26    // Maximum number of columns to display
 #define MAX_MENU_ITEMS       5     // Maximum number of menu items
 #define CURSOR_BLINK_RATE    10    // Number of frames between cursor blinks
 
 /* ============================== Function Prototypes ============================== */
 
 // Main UI function
 void run_calculator_ui(void);
 
 // UI components
 void draw_header(void);
 void show_input_prompt(void);
 void show_calculation_result(CalculationResult* result);
 void show_error_message(const char* message);
 void show_help_screen(void);
 void show_settings_menu(void);
 
 // Input handling
 bool get_expression_input(char* buffer, int buffer_size);
 void handle_keys(void);
 
 // Settings
 void toggle_arithmetic_mode(void);
 void adjust_precision(int change);
 void toggle_significant_digits(void);
 
 #endif /* UI_H */
 