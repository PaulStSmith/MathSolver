#include <tice.h>
#include <graphx.h>
#include <string.h>
#include "headers/gui_private.h"

#define LOG_TAG "GUI"
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
    // log_debuglog_debug("GUI_get_settings() called");  /**< Log the function call */
    return &_settings_;
}

/**
 * @brief Initializes the GUI system and sets default configurations.
 */
void GUI_init(void) {
    logger_init();
    // log_debuglog_debug("GUI_init() called");  /**< Log the function call */
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_FillScreen(BG_COLOR);
    gfx_SetTextFGColor(TEXT_COLOR);
    gfx_SetTextBGColor(BG_COLOR);
    gfx_SetMonospaceFont(GUI_CHAR_WIDTH); 
    gfx_SetFontHeight(GUI_CHAR_HEIGHT); 
}

/**
 * @brief Ends the GUI system and releases resources.
 */
void GUI_end(void) {
    // log_debuglog_debug("GUI_end() called");
    gfx_End();
    logger_close();
}

/**
 * @brief Sets the background color for the GUI.
 * @param color The new background color (0-255).
 */
void GUI_set_bg_color(int color) {
    // log_debuglog_debug("GUI_set_bg_color() called");  /**< Log the function call */
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
    // log_debuglog_debug("GUI_set_text_color() called");  /**< Log the function call */
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
    // log_debuglog_debug("GUI_set_highlight_color() called");  /**< Log the function call */
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
    // log_debuglog_debug("GUI_set_colors() called");  /**< Log the function call */
    GUI_set_bg_color(bg_color);
    GUI_set_text_color(text_color);
    GUI_set_highlight_color(highlight_color);
}

/**
 * @brief Resets all colors to their default values.
 */
void GUI_reset_colors(void) {
    // log_debuglog_debug("GUI_reset_colors() called");  /**< Log the function call */
    GUI_set_colors(COLOR_WHITE, COLOR_BLACK, 0x10);
}

/**
 * @brief Sets the text foreground and background colors.
 * @param text_color The new text color.
 * @param bg_color The new background color.
 */
void GUI_set_text_colors(int text_color, int bg_color) {
    // log_debuglog_debug("GUI_set_text_colors() called");  /**< Log the function call */
    GUI_set_bg_color(bg_color);
    GUI_set_text_color(text_color);
}

/**
 * @brief Resets the text colors to their default values.
 */
void GUI_reset_text_colors(void) {
    // log_debuglog_debug("GUI_reset_text_colors() called");  /**< Log the function call */
    GUI_set_text_colors(COLOR_BLACK, COLOR_WHITE);
}

/**
 * @brief Writes text at the specified (x, y) position.
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param text The text to write.
 */
void GUI_write_text(int x, int y, const char* text) {
    // log_debuglog_debug("GUI_write_text() called");  /**< Log the function call */
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
    // log_debuglog_debug("GUI_write_text_centered() called");  /**< Log the function call */
    int x = (LCD_WIDTH - gfx_GetStringWidth(text)) / 2; /**< Calculate the x-coordinate for centering */
    GUI_write_text(x, y, text);
}

/**
 * @brief Writes text aligned to the right at the specified y-coordinate.
 * @param y The y-coordinate.
 * @param text The text to write.
 */
void GUI_write_text_right(int y, const char* text) {
    // log_debuglog_debug("GUI_write_text_right() called");  /**< Log the function call */
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
    // log_debuglog_debug("GUI_print_text_column() called");
    
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
    // log_debuglog_debug("GUI_print_text() called");
    
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
    // log_debuglog_debug("_GUI_print_text_internal() called");
    
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
        gfx_PrintStringXY(chunk, x, y);
        
        // Move to next line
        y += GUI_LINE_HEIGHT;
        x = reset_x ? original_x : PADDING_X;
        
        // Check for bottom of screen
        if (y >= LCD_HEIGHT) {
            break;  // Stop if we've reached bottom of screen
        }
    }
}

// Default initial buffer size
#define INITIAL_BUFFER_SIZE 64

/**
 * @brief Initialize a horizontally scrollable text field.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param x X position
 * @param y Y position
 * @param width Width of text field in pixels
 * @param has_border Whether to draw a border
 */
void GUI_hscroll_init(HScrollTextField* field, int x, int y, int width, bool has_border) {
    // log_debuglog_debug("GUI_hscroll_init() called");
    
    memset(field, 0, sizeof(HScrollTextField));
    field->x = x;
    field->y = y;
    field->width = width;
    field->has_border = has_border;
    
    // Calculate maximum visible characters
    field->max_visible_chars = (width - 2*PADDING_X) / GUI_CHAR_WIDTH;
    
    // Allocate initial buffer
    field->buffer_size = INITIAL_BUFFER_SIZE;
    field->text = (char*)malloc(field->buffer_size);
    if (field->text) {
        field->text[0] = '\0';
    } else {
        log_error("Failed to allocate text buffer");
    }
    
    // Initialize other values
    field->text_length = 0;
    field->cursor_position = 0;
    field->scroll_offset = 0;
    field->is_active = false;
}

/**
 * @brief Free resources used by a text field.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_free(HScrollTextField* field) {
    // log_debuglog_debug("GUI_hscroll_free() called");
    
    if (field->text) {
        free(field->text);
        field->text = NULL;
    }
    field->buffer_size = 0;
    field->text_length = 0;
}

/**
 * @brief Ensure the buffer is large enough for the given size.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param needed_size Size needed including null terminator
 * @return true if buffer is large enough or was successfully resized
 */
static bool ensure_buffer_size(HScrollTextField* field, int needed_size) {
    if (!field->text) return false;
    
    if (needed_size <= field->buffer_size) {
        return true; // Already large enough
    }
    
    // Calculate new size (double until sufficient)
    int new_size = field->buffer_size;
    while (new_size < needed_size) {
        new_size *= 2;
    }
    
    // Reallocate
    char* new_buffer = (char*)realloc(field->text, new_size);
    if (!new_buffer) {
        log_error("Failed to resize text buffer");
        return false;
    }
    
    field->text = new_buffer;
    field->buffer_size = new_size;
    return true;
}

/**
 * @brief Set the text content of the field.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param text Text to set
 */
void GUI_hscroll_set_text(HScrollTextField* field, const char* text) {
    // log_debuglog_debug("GUI_hscroll_set_text() called");
    
    int length = strlen(text);
    if (ensure_buffer_size(field, length + 1)) {
        strcpy(field->text, text);
        field->text_length = length;
        
        // Reset cursor and scroll
        field->cursor_position = length;
        GUI_hscroll_ensure_cursor_visible(field);
    }
}

/**
 * @brief Clear the text content of the field.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_clear(HScrollTextField* field) {
    // log_debuglog_debug("GUI_hscroll_clear() called");
    
    if (field->text) {
        field->text[0] = '\0';  // Clear text
        field->text_length = 0;
        field->cursor_position = 0;
        field->scroll_offset = 0;
    }
}

/**
 * @brief Append text to the current content.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param text Text to append
 */
void GUI_hscroll_append(HScrollTextField* field, const char* text) {
    // log_debuglog_debug("GUI_hscroll_append() called");
    
    int append_length = strlen(text);
    int new_length = field->text_length + append_length;
    
    if (ensure_buffer_size(field, new_length + 1)) {
        strcat(field->text, text);
        field->text_length = new_length;
        
        // Move cursor to end
        field->cursor_position = new_length;
        GUI_hscroll_ensure_cursor_visible(field);
    }
}

/**
 * @brief Insert a character at the current cursor position.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param c Character to insert
 */
void GUI_hscroll_insert_char(HScrollTextField* field, char c) {
    // log_debuglog_debug("GUI_hscroll_insert_char() called");
    
    if (ensure_buffer_size(field, field->text_length + 2)) {
        // Shift characters after cursor
        for (int i = field->text_length; i >= field->cursor_position; i--) {
            field->text[i + 1] = field->text[i];
        }
        
        // Insert the new character
        field->text[field->cursor_position] = c;
        field->text_length++;
        field->cursor_position++;
        
        GUI_hscroll_ensure_cursor_visible(field);
    }
}

/**
 * @brief Delete the character before the cursor (backspace).
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_delete_char(HScrollTextField* field) {
    // log_debuglog_debug("GUI_hscroll_delete_char() called");
    
    if (field->cursor_position > 0) {
        // Shift characters at and after cursor
        for (int i = field->cursor_position - 1; i < field->text_length; i++) {
            field->text[i] = field->text[i + 1];
        }
        
        field->text_length--;
        field->cursor_position--;
        
        GUI_hscroll_ensure_cursor_visible(field);
    }
}

/**
 * @brief Move cursor left one character.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_cursor_left(HScrollTextField* field) {
    if (field->cursor_position > 0) {
        field->cursor_position--;
        GUI_hscroll_ensure_cursor_visible(field);
    }
}

/**
 * @brief Move cursor right one character.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_cursor_right(HScrollTextField* field) {
    if (field->cursor_position < field->text_length) {
        field->cursor_position++;
        GUI_hscroll_ensure_cursor_visible(field);
    }
}

/**
 * @brief Move cursor to the start of the text.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_cursor_to_start(HScrollTextField* field) {
    field->cursor_position = 0;
    field->scroll_offset = 0;
}

/**
 * @brief Move cursor to the end of the text.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_cursor_to_end(HScrollTextField* field) {
    field->cursor_position = field->text_length;
    GUI_hscroll_ensure_cursor_visible(field);
}

/**
 * @brief Scroll the text view left.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param chars Number of characters to scroll
 */
void GUI_hscroll_scroll_left(HScrollTextField* field, int chars) {
    field->scroll_offset -= chars;
    if (field->scroll_offset < 0) {
        field->scroll_offset = 0;
    }
}

/**
 * @brief Scroll the text view right.
 * 
 * @param field Pointer to HScrollTextField structure
 * @param chars Number of characters to scroll
 */
void GUI_hscroll_scroll_right(HScrollTextField* field, int chars) {
    int max_offset = field->text_length - field->max_visible_chars;
    if (max_offset < 0) max_offset = 0;
    
    field->scroll_offset += chars;
    if (field->scroll_offset > max_offset) {
        field->scroll_offset = max_offset;
    }
}

/**
 * @brief Ensure the cursor is visible in the current scroll view.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_ensure_cursor_visible(HScrollTextField* field) {
    // If cursor is to the left of visible area
    if (field->cursor_position < field->scroll_offset) {
        field->scroll_offset = field->cursor_position;
    }
    
    // If cursor is to the right of visible area
    else if (field->cursor_position >= field->scroll_offset + field->max_visible_chars) {
        field->scroll_offset = field->cursor_position - field->max_visible_chars + 1;
    }
}

/**
 * @brief Draw the scrollable text field to the buffer.
 * 
 * @param field Pointer to HScrollTextField structure
 */
void GUI_hscroll_draw(HScrollTextField* field) {
    // log_debuglog_debug("GUI_hscroll_draw() called");
    GUISettings* settings = GUI_get_settings();
    
    // Draw background and border
    if (field->has_border) {
        // Draw border rectangle
        gfx_SetColor(settings->text_color);
        gfx_Rectangle(field->x, field->y, field->width, GUI_LINE_HEIGHT + 2*PADDING_Y);
        
        // Fill inner area
        gfx_SetColor(settings->bg_color);
        gfx_FillRectangle(field->x + 1, field->y + 1, 
                          field->width - 2, GUI_LINE_HEIGHT + 2*PADDING_Y - 2);
    } else {
        // Just fill the entire area
        gfx_SetColor(settings->bg_color);
        gfx_FillRectangle(field->x, field->y, field->width, GUI_LINE_HEIGHT + 2*PADDING_Y);
    }
    
    // Calculate visible area
    int content_x = field->x + PADDING_X;
    int content_y = field->y + PADDING_Y;
    
    // Create a temporary buffer for the visible text
    char visible_text[128]; // Assuming max_visible_chars is less than 128
    
    // Extract visible portion of the text
    int visible_length = field->text_length - field->scroll_offset;
    if (visible_length > field->max_visible_chars) {
        visible_length = field->max_visible_chars;
    }
    
    if (visible_length > 0) {
        strncpy(visible_text, field->text + field->scroll_offset, visible_length);
    }
    visible_text[visible_length] = '\0';
    
    // Draw the visible text
    gfx_SetTextFGColor(settings->text_color);
    gfx_SetTextBGColor(settings->bg_color);
    gfx_PrintStringXY(visible_text, content_x, content_y);
    
    // Draw cursor if active
    if (field->is_active) {
        int cursor_x = content_x + (field->cursor_position - field->scroll_offset) * GUI_CHAR_WIDTH;
        gfx_SetColor(settings->text_color);
        gfx_Line(cursor_x, content_y, cursor_x, content_y + GUI_CHAR_HEIGHT);
    }
    
    // Draw scroll indicators if needed
    if (field->scroll_offset > 0) {
        // Left scroll indicator (e.g., a small triangle or '<')
        gfx_SetTextFGColor(settings->text_color);
        gfx_PrintStringXY("<", field->x, content_y);
    }
    
    if (field->scroll_offset + field->max_visible_chars < field->text_length) {
        // Right scroll indicator
        gfx_SetTextFGColor(settings->text_color);
        gfx_PrintStringXY(">", field->x + field->width - 8, content_y);
    }
}
