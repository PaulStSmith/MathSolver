#include <tice.h>
#include <graphx.h>
#include <string.h>
#include "headers/gui_private.h"

#include "headers/log.h"

/**
 * @brief Structure to hold GUI settings such as background color, text color, and highlight color.
 */
static GUISettings _settings_ = {
    .bg_color = COLOR_WHITE,       /**< Default background color */
    .text_color = COLOR_BLACK,     /**< Default text color */
    .highlight_color = 0x10        /**< Default highlight color */
};

/**
 * @brief Returns a pointer to the current GUI settings.
 * @return Pointer to the GUISettings structure.
 */
GUISettings* GUI_get_settings(void) {
        return &_settings_;
}

/**
 * @brief Initializes the GUI system and sets default configurations.
 */
void GUI_init(void) {
        gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_FillScreen(BG_COLOR);
    gfx_SetTextFGColor(TEXT_COLOR);
    gfx_SetTextBGColor(BG_COLOR);
    gfx_SetMonospaceFont(GUI_CHAR_WIDTH); 
    gfx_SetFontHeight(GUI_CHAR_HEIGHT); 
}

void GUI_refresh(void) {
    gfx_BlitBuffer();  /**< Swap the draw buffer to display the changes */
}

/**
 * @brief Ends the GUI system and releases resources.
 */
void GUI_end(void) {
        gfx_End();
}

/**
 * @brief Sets the background color for the GUI.
 * @param color The new background color (0-255).
 */
void GUI_set_bg_color(int color) {
        if (color < 0 || color > 255) {
        color = COLOR_WHITE; /**< Invalid color value, set to default */
    }
    _settings_.bg_color = color;
}

/**
 * @brief Sets the text color for the GUI.
 * @param color The new text color (0-255).
 */
void GUI_set_text_color(int color) {
        if (color < 0 || color > 255) {
        color = COLOR_BLACK; /**< Invalid color value, set to default */
    }
    _settings_.text_color = color;
}

/**
 * @brief Sets the highlight color for the GUI.
 * @param color The new highlight color (0-255).
 */
void GUI_set_highlight_color(int color) {
        if (color < 0 || color > 255) {
        color = 0x10; /**< Invalid color value, set to default */
    }
    _settings_.highlight_color = color;
}

/**
 * @brief Sets all colors (background, text, and highlight) for the GUI.
 * @param bg_color The new background color.
 * @param text_color The new text color.
 * @param highlight_color The new highlight color.
 */
void GUI_set_colors(int bg_color, int text_color, int highlight_color) {
        GUI_set_bg_color(bg_color);
    GUI_set_text_color(text_color);
    GUI_set_highlight_color(highlight_color);
}

/**
 * @brief Resets all colors to their default values.
 */
void GUI_reset_colors(void) {
        GUI_set_colors(COLOR_WHITE, COLOR_BLACK, 0x10);
}

/**
 * @brief Sets the text foreground and background colors.
 * @param text_color The new text color.
 * @param bg_color The new background color.
 */
void GUI_set_text_colors(int text_color, int bg_color) {
        GUI_set_bg_color(bg_color);
    GUI_set_text_color(text_color);
}

/**
 * @brief Resets the text colors to their default values.
 */
void GUI_reset_text_colors(void) {
        GUI_set_text_colors(COLOR_BLACK, COLOR_WHITE);
}

/**
 * @brief Writes text at the specified (x, y) position.
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param text The text to write.
 */
void GUI_write_text(int x, int y, const char* text) {
        gfx_SetTextFGColor(_settings_.text_color);  /**< Set the text foreground color */
    gfx_SetTextBGColor(_settings_.bg_color);    /**< Set the text background color */
    gfx_PrintStringXY(text, x, y);              /**< Print the string at the specified position */
}

/**
 * @brief Writes text centered horizontally at the specified y-coordinate.
 * @param y The y-coordinate.
 * @param text The text to write.
 */
void GUI_write_text_centered(int y, const char* text) {
        int x = (LCD_WIDTH - gfx_GetStringWidth(text)) / 2; /**< Calculate the x-coordinate for centering */
    GUI_write_text(x, y, text);
}

/**
 * @brief Writes text aligned to the right at the specified y-coordinate.
 * @param y The y-coordinate.
 * @param text The text to write.
 */
void GUI_write_text_right(int y, const char* text) {
        int x = LCD_WIDTH - gfx_GetStringWidth(text) - PADDING_X; /**< Calculate the x-coordinate for right alignment */
    GUI_write_text(x, y, text);
}

/**
 * @brief Writes text aligned to the left at the specified y-coordinate.
 * @param y The y-coordinate.
 * @param text The text to write.
 */
void GUI_write_text_left(int y, const char* text) {
    GUI_write_text(PADDING_X, y, text); /**< Use a fixed padding for left alignment */
}

/**
 * @brief Prints text at the specified (x, y) position, wrapping to the next line by chunking.
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param text The text to print.
 */
void GUI_print_text_column(int x, int y, const char* text) {
        
    // Calculate max width from x to edge of screen
    int max_width = LCD_WIDTH - x;
    
    // Call internal function with column-specific parameters
    _GUI_print_text_internal(x, y, text, max_width, true);
}

/**
 * @brief Prints text at the specified (x, y) position, flowing through the entire screen width.
 * @param x The starting x-coordinate.
 * @param y The starting y-coordinate.
 * @param text The text to print.
 */
void GUI_print_text(int x, int y, const char* text) {
        
    // Log text length for debugging
    log_message("Text length: %d", strlen(text));
    
    // Call internal function with full-width parameters
    _GUI_print_text_internal(x, y, text, -1, false);
}

/**
 * @brief Internal function to print text with customizable wrapping behavior.
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param text The text to print.
 * @param max_width Maximum width to use for line calculation, or -1 to use full screen width.
 * @param reset_x If true, each new line starts at the original x; if false, new lines start at PADDING_X.
 */
static void _GUI_print_text_internal(int x, int y, const char* text, int max_width, bool reset_x) {
        
    // Setup
    int original_x = x;               // Save original x for new lines
    int text_length = strlen(text);   // Total text length
    int curr_pos = 0;                 // Current position in text
    char chunk[256];                  // Buffer for text chunks
    
    // Set text display properties
    gfx_SetTextFGColor(_settings_.text_color);
    gfx_SetTextBGColor(_settings_.bg_color);
    
    // First line might be shorter if x doesn't start at 0
    int available_width = (max_width > 0) ? max_width : (LCD_WIDTH - PADDING_X);
    int first_line_chars = (available_width - x + PADDING_X) / GUI_CHAR_WIDTH;
    int full_line_chars = (available_width - PADDING_X) / GUI_CHAR_WIDTH;
    
    // Process text in chunks
    while (curr_pos < text_length) {
        // Determine how many characters to print on this line
        int chars_this_line = (curr_pos == 0) ? first_line_chars : full_line_chars;
        if (chars_this_line > 255) chars_this_line = 255;  // Buffer safety
        
        // Don't read past the end of the string
        if (curr_pos + chars_this_line > text_length) {
            chars_this_line = text_length - curr_pos;
        }
        
        // Extract chunk
        memset(chunk, 0, sizeof(chunk));  // Clear buffer
        strncpy(chunk, text + curr_pos, chars_this_line);
        chunk[chars_this_line] = '\0';  // Ensure null termination
        
        // Handle newlines in the chunk
        char* newline = strchr(chunk, '\n');
        if (newline) {
            *newline = '\0';  // Terminate at newline
            curr_pos += (newline - chunk + 1);  // +1 to skip the newline
        } else {
            curr_pos += chars_this_line;  // Move to next chunk
        }
        
        // Print the chunk
       GUI_put_text(x, y, chunk); 
        
        // Move to next line
        y += GUI_LINE_HEIGHT;
        x = reset_x ? original_x : PADDING_X;
        
        // Check for bottom of screen
        if (y >= LCD_HEIGHT) {
            break;  // Stop if we've reached bottom of screen
        }
    }
}

static void GUI_put_text(int x, int y, const char* text) {
    GUISettings* settings = GUI_get_settings();

    int width = gfx_GetStringWidth(text);               /**< Get the width of the string */
    
    gfx_SetColor(settings->bg_color);
    gfx_FillRectangle(x, y, width, GUI_LINE_HEIGHT);    /**< Clear the area where the text will be printed */

    gfx_SetColor(_settings_.text_color);                /**< Set the text color */
    gfx_PrintStringXY(text, x, y);                      /**< Print the string at the specified position */
}
