#include <tice.h>
#include <graphx.h>
#include "headers/gui_private.h"

static GUISettings _settings_ = {
    .bg_color = COLOR_WHITE,
    .text_color = COLOR_BLACK,
    .highlight_color = 0x10
};

GUISettings* GUI_get_settings(void) {
    return &_settings_;
}

void GUI_init(void) {
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_FillScreen(BG_COLOR);
    gfx_SetTextFGColor(_settings_.text_color);
    gfx_SetTextBGColor(_settings_.bg_color);
    gfx_SetTextConfig(gfx_text_noclip);
    gfx_SetTextTransparentColor(_settings_.bg_color);
    gfx_SetPalette(NULL, 0, 0);                         // Reset palette to default
    gfx_SetMonospaceFont(GUI_CHAR_WIDTH);               // Set the font width
    gfx_SetFontHeight(GUI_CHAR_HEIGHT);                 // Set font height
}

void GUI_end(void) {
    gfx_End();
}

void GUI_set_bg_color(int color) {
    if (color < 0 || color > 255) {
        // Invalid color value, set to default
        color = COLOR_WHITE;
    }
    _settings_.bg_color = color;
}

void GUI_set_text_color(int color) {
    if (color < 0 || color > 255) {
        // Invalid color value, set to default
        color = COLOR_BLACK;
    }
    _settings_.text_color = color;
}
void GUI_set_highlight_color(int color) {
    if (color < 0 || color > 255) {
        // Invalid color value, set to default
        color = 0x10; // Default highlight color
    }
    _settings_.highlight_color = color;
}

void GUI_set_colors(int bg_color, int text_color, int highlight_color) {
    GUI_set_bg_color(bg_color);
    GUI_set_text_color(text_color);
    GUI_set_highlight_color(highlight_color);
}
void GUI_reset_colors(void) {
    GUI_set_colors(COLOR_WHITE, COLOR_BLACK, 0x10);
}

void GUI_set_text_colors(int text_color, int bg_color) {
    GUI_set_bg_color(bg_color);
    GUI_set_text_color(text_color);
}

void GUI_reset_text_colors(void) {
    GUI_set_text_colors(COLOR_BLACK, COLOR_WHITE);
}

void GUI_write_text(int x, int y, const char* text) {
    gfx_SetTextFGColor(_settings_.text_color);
    gfx_SetTextBGColor(_settings_.bg_color);
    gfx_PrintStringXY(text, x, y);
}
void GUI_write_text_centered(int y, const char* text) {
    int x = (LCD_WIDTH - gfx_GetStringWidth(text)) / 2;
    GUI_write_text(x, y, text);
}

void GUI_write_text_right(int y, const char* text) {
    int x = LCD_WIDTH - gfx_GetStringWidth(text) - PADDING_X;
    // Negative values of x simply mean that the 
    // text will be clipped on the left side.
    GUI_write_text(x, y, text);
}
void GUI_write_text_left(int y, const char* text) {
    GUI_write_text(PADDING_X, y, text);
}

void GUI_print_text(int x, int y, const char* text) {
    gfx_SetTextFGColor(_settings_.text_color);
    gfx_SetTextBGColor(_settings_.bg_color);

    /*
     * Calculate the width of the text to be printed
     */
    int text_width = gfx_GetStringWidth(text);
    int printed_width = 0;
    int visible_width = LCD_WIDTH - x;
    while (printed_width < text_width) {
        GUI_print_text(x, y, text);
        x = -printed_width;
        printed_width += visible_width;
        visible_width = LCD_WIDTH;
        y += GUI_CHAR_HEIGHT;
        if (y >= LCD_HEIGHT) {
            break; // Stop if we reach the bottom of the screen
        }
    }
    gfx_PrintStringXY(text, x, y);
}