#ifndef GUI_H
#define GUI_H

/**
 * @file gui.h
 * @brief Header file for GUI-related constants, macros, and structures.
 * 
 * This file defines color constants, GUI layout parameters, and a structure
 * for GUI settings. It is used to configure and manage the graphical interface
 * of the application.
 */

// PC DOS basic 16 colors mapped to TI-84 CE palette
/**
 * @brief Color definitions mapped to the TI-84 CE palette.
 */
#define COLOR_BLACK         0x00  /**< Black */
#define COLOR_BLUE          0x03  /**< Blue */
#define COLOR_GREEN         0x1C  /**< Green */
#define COLOR_CYAN          0x1F  /**< Cyan */
#define COLOR_RED           0xE0  /**< Red */
#define COLOR_MAGENTA       0xE3  /**< Magenta */
#define COLOR_BROWN         0xA0  /**< Brown (approximation) */
#define COLOR_LIGHT_GRAY    0x77  /**< Light Gray (approximation) */
#define COLOR_DARK_GRAY     0x55  /**< Dark Gray (approximation) */
#define COLOR_LIGHT_BLUE    0x7F  /**< Light Blue (approximation) */
#define COLOR_LIGHT_GREEN   0x3E  /**< Light Green (approximation) */
#define COLOR_LIGHT_CYAN    0x7F  /**< Light Cyan (approximation) */
#define COLOR_LIGHT_RED     0xF4  /**< Light Red (approximation) */
#define COLOR_LIGHT_MAGENTA 0xF7  /**< Light Magenta (approximation) */
#define COLOR_YELLOW        0xFC  /**< Yellow */
#define COLOR_WHITE         0xFF  /**< White */

// Colors for the graphical interface
/**
 * @brief Default colors for the graphical interface.
 */
#define BG_COLOR            COLOR_WHITE /**< Background color */
#define TEXT_COLOR          COLOR_BLACK /**< Text color */
#define HIGHLIGHT_COLOR     0x10        /**< Highlight color (custom value) */

// Layout parameters
/**
 * @brief Padding and character dimensions for the GUI.
 */
#define PADDING_X          3           /**< Horizontal padding for text */
#define PADDING_Y          3           /**< Vertical padding for text */
#define GUI_CHAR_WIDTH     8           /**< Width of a character in pixels */
#define GUI_CHAR_HEIGHT    8           /**< Height of a character in pixels */
#define GUI_LINE_HEIGHT    9           /**< Height of a line in pixels */

/**
 * @brief Maximum number of characters per line and lines per screen.
 * 
 * These macros calculate the maximum number of characters and lines
 * that can fit on the screen based on the LCD dimensions and padding.
 */
#define GUI_MAX_CHARS_PER_LINE ((LCD_WIDTH - PADDING_X) / GUI_CHAR_WIDTH) /**< Max chars per line */
#define GUI_MAX_LINES      ((LCD_HEIGHT - PADDING_Y) / GUI_LINE_HEIGHT)   /**< Max lines on screen */

/**
 * @struct GUISettings
 * @brief Structure to hold GUI settings.
 * 
 * This structure contains the background color, text color, and highlight
 * color for the graphical interface.
 */
typedef struct {
    int bg_color;         /**< Background color */
    int text_color;       /**< Text color */
    int highlight_color;  /**< Highlight color */
} GUISettings;

#include "gui_public.h"

#endif // GUI_H
