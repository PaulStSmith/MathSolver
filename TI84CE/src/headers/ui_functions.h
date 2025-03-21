
/*
 * This is an automatically generated header file from the source file: .\src\ui.c
 * Do not modify this file directly. Instead, modify the source file and regenerate this header.
 * 
 * Generated on: 03/21/2025 11:55:27
 * Source file size: 17.38 KB
 * Source file lines: 567
 * Function prototypes found: 20
 */

#ifndef UI_FUNCTIONS_H
#define UI_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

// Function Prototypes
static void clear_screen(void);
static void new_line(void);
static void print_string(const char* str);
static void print_centered(const char* str);
static void print_truncated(const char* str, int max_length);
static void print_format(const char* format, ...);
static void print_format_centered(const char* format, ...);
static void print_format_truncated(int max_length, const char* format, ...);
static void draw_horizontal_line(char line_char);
void draw_header(void);
void show_input_prompt(void);
void show_calculation_result(CalculationResult* result);
void show_error_message(const char* message);
void show_help_screen(void);
void show_settings_menu(void);
bool get_expression_input(char* buffer, int buffer_size);
void toggle_arithmetic_mode(void);
void adjust_precision(int change);
void toggle_significant_digits(void);
void run_calculator_ui(void);

#ifdef __cplusplus
}
#endif

#endif // UI_FUNCTIONS_H
