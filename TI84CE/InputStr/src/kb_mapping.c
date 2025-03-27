#include <string.h>
#include <stdio.h>
#include "headers/kb_mapping.h"

/**
 * Initialize the keyboard state tracker
 */
KeyboardState key_mapping_init(void) {
    KeyboardState state;
    state.current_mode = KB_MODE_NORMAL;
    state.alpha_active = false;
    state.second_active = false;
    state.alpha_lock = false;
    return state;
}

/**
 * Process mode keys (2nd, Alpha) and update keyboard state
 */
bool key_mapping_process_mode_keys(CombinedKey key, KeyboardState* state) {
    if (!state) return false;

    // Check for mode keys
    if (key == MAKE_KEY(2, kb_Alpha)) {
        // Alpha key pressed
        if (state->second_active) {
            // 2nd+Alpha = Alpha Lock mode
            state->alpha_active = true;
            state->alpha_lock = true;
            state->second_active = false;
            state->current_mode = KB_MODE_ALPHA_LOCK;
        } else if (state->alpha_lock) {
            // If in alpha lock, pressing Alpha exits alpha lock
            state->alpha_active = false;
            state->alpha_lock = false;
            state->current_mode = KB_MODE_NORMAL;
        } else {
            // Regular Alpha key (temporary alpha mode)
            state->alpha_active = !state->alpha_active;
            state->current_mode = state->alpha_active ? KB_MODE_ALPHA : KB_MODE_NORMAL;
        }
        return true;
    } else if (key == MAKE_KEY(1, kb_2nd)) {
        // 2nd key pressed - toggle 2nd mode
        state->second_active = !state->second_active;
        state->current_mode = state->second_active ? KB_MODE_2ND : KB_MODE_NORMAL;
        return true;
    } else if (state->current_mode == KB_MODE_ALPHA) {
        state->alpha_active = false;
        state->current_mode = KB_MODE_NORMAL;
    }

    return false;
}

/**
 * Gets the character or function identifier for a key based on current keyboard mode.
 */
int key_mapping_get_value(CombinedKey key, KeyboardState state) {
    int group = KEY_GROUP(key);
    int mask = KEY_MASK(key);
    bool is_alpha = state.alpha_active;
    bool is_2nd = state.second_active;

    // Handle normal mode (no alpha, no 2nd)
    if (!is_alpha && !is_2nd) {
        // Group 1: Graph, Trace, Zoom, Window, Y=, 2nd, Mode, Del
        if (group == 1) {
            if (mask == kb_Graph) return FUNC_GRAPH;
            if (mask == kb_Trace) return FUNC_TRACE;
            if (mask == kb_Zoom) return FUNC_ZOOM;
            if (mask == kb_Window) return FUNC_WINDOW;
            if (mask == kb_Yequ) return FUNC_Y_EQUALS;
            if (mask == kb_2nd) return KB_KEY_2ND;     // Should be processed by key_mapping_process_mode_keys
            if (mask == kb_Mode) return KB_KEY_MODE;
            if (mask == kb_Del) return KB_KEY_DEL;
        }
        // Group 2: Sto, Ln, Log, x², 1/x, Math, Alpha
        else if (group == 2) {
            if (mask == kb_Sto) return FUNC_STO;
            if (mask == kb_Ln) return FUNC_LN;
            if (mask == kb_Log) return FUNC_LOG;
            if (mask == kb_Square) return FUNC_SQUARE;
            if (mask == kb_Recip) return FUNC_RECIP;
            if (mask == kb_Math) return FUNC_MATH;
            if (mask == kb_Alpha) return KB_KEY_ALPHA; // Should be processed by key_mapping_process_mode_keys
        }
        // Group 3: 0, 1, 4, 7, ,, sin, apps, x
        else if (group == 3) {
            if (mask == kb_0) return '0';
            if (mask == kb_1) return '1';
            if (mask == kb_4) return '4';
            if (mask == kb_7) return '7';
            if (mask == kb_Comma) return ',';
            if (mask == kb_Sin) return FUNC_SIN;
            if (mask == kb_Apps) return FUNC_APPS;
            if (mask == kb_GraphVar) return FUNC_X_VAR;
        }
        // Group 4: ., 2, 5, 8, (, cos, prgm, stat
        else if (group == 4) {
            if (mask == kb_DecPnt) return '.';
            if (mask == kb_2) return '2';
            if (mask == kb_5) return '5';
            if (mask == kb_8) return '8';
            if (mask == kb_LParen) return '(';
            if (mask == kb_Cos) return FUNC_COS;
            if (mask == kb_Prgm) return FUNC_PRGM;
            if (mask == kb_Stat) return FUNC_STAT;
        }
        // Group 5: (-), 3, 6, 9, ), tan, vars
        else if (group == 5) {
            if (mask == kb_Chs) return FUNC_PI; // This is a bit unusual, but often (-) is used for π in normal mode
            if (mask == kb_3) return '3';
            if (mask == kb_6) return '6';
            if (mask == kb_9) return '9';
            if (mask == kb_RParen) return ')';
            if (mask == kb_Tan) return FUNC_TAN;
            if (mask == kb_Vars) return FUNC_VARS;
        }
        // Group 6: Enter, +, -, *, /, ^, clear
        else if (group == 6) {
            if (mask == kb_Enter) return KB_KEY_ENTER;
            if (mask == kb_Add) return '+';
            if (mask == kb_Sub) return '-';
            if (mask == kb_Mul) return '*';
            if (mask == kb_Div) return '/';
            if (mask == kb_Power) return '^';
            if (mask == kb_Clear) return KB_KEY_CLEAR;
        }
        // Group 7: down, left, right, up
        else if (group == 7) {
            if (mask == kb_Down) return KB_KEY_DOWN;
            if (mask == kb_Left) return KB_KEY_LEFT;
            if (mask == kb_Right) return KB_KEY_RIGHT;
            if (mask == kb_Up) return KB_KEY_UP;
        }
    }
    // Handle alpha mode
    else if (is_alpha && !is_2nd) {
        // This maps the green letters on the calculator
        // Group 3: Space, A, D, G
        if (group == 3) {
            if (mask == kb_0) return ' '; // Space
            if (mask == kb_1) return 'A';
            if (mask == kb_4) return 'D';
            if (mask == kb_7) return 'G';
            if (mask == kb_Comma) return '"'; // Quote
            if (mask == kb_Sin) return 'X';
            if (mask == kb_Apps) return 'O';
            if (mask == kb_GraphVar) return 'T';
        }
        // Group 4: Period, B, E, H
        else if (group == 4) {
            if (mask == kb_DecPnt) return ':'; // Colon
            if (mask == kb_2) return 'B';
            if (mask == kb_5) return 'E';
            if (mask == kb_8) return 'H';
            if (mask == kb_LParen) return '[';
            if (mask == kb_Cos) return 'Y';
            if (mask == kb_Prgm) return 'P';
            if (mask == kb_Stat) return 'Z';
        }
        // Group 5: ?, C, F, I
        else if (group == 5) {
            if (mask == kb_Chs) return '?';
            if (mask == kb_3) return 'C';
            if (mask == kb_6) return 'F';
            if (mask == kb_9) return 'I';
            if (mask == kb_RParen) return ']';
            if (mask == kb_Tan) return 'U';
            if (mask == kb_Vars) return 'Q';
        }
        // Group 2: Strings and other characters
        else if (group == 2) {
            if (mask == kb_Sto) return '~';
            if (mask == kb_Ln) return 'J';
            if (mask == kb_Log) return 'K';
            if (mask == kb_Square) return 'L';
            if (mask == kb_Recip) return 'M';
            if (mask == kb_Math) return 'N';
        }
        // Group 1: More strings
        else if (group == 1) {
            if (mask == kb_Graph) return '`'; // Grave accent
            if (mask == kb_Trace) return '=';
            if (mask == kb_Zoom) return '\\';
            if (mask == kb_Window) return '_'; // Underscore
            if (mask == kb_Yequ) return '<';
            if (mask == kb_Del) return '>';
        }
        // Group 6: Standard operations
        else if (group == 6) {
            if (mask == kb_Enter) return KB_KEY_ENTER;
            if (mask == kb_Add) return '+';
            if (mask == kb_Sub) return '-';
            if (mask == kb_Mul) return '*';
            if (mask == kb_Div) return '/';
            if (mask == kb_Power) return '^';
            if (mask == kb_Clear) return KB_KEY_CLEAR;
        }
        // Group 7: Arrow keys - keep the same
        else if (group == 7) {
            if (mask == kb_Down) return KB_KEY_DOWN;
            if (mask == kb_Left) return KB_KEY_LEFT;
            if (mask == kb_Right) return KB_KEY_RIGHT;
            if (mask == kb_Up) return KB_KEY_UP;
        }
    }
    // Handle 2nd mode
    else if (!is_alpha && is_2nd) {
        // This maps to the blue functions on the calculator
        // Group 1: F1-F5 functions and catalog
        if (group == 1) {
            if (mask == kb_Graph) return FUNC_TABLE;    // F5
            if (mask == kb_Trace) return FUNC_CALC;     // F4
            if (mask == kb_Zoom) return FUNC_FORMAT;    // F3
            if (mask == kb_Window) return FUNC_TBLSET;  // F2
            if (mask == kb_Yequ) return FUNC_STATPLOT; // F1
            if (mask == kb_Mode) return FUNC_QUIT;
            if (mask == kb_Del) return FUNC_INS;
        }
        // Group 2: Other 2nd functions
        else if (group == 2) {
            if (mask == kb_Sto) return FUNC_RECALL;
            if (mask == kb_Ln) return FUNC_EXP;
            if (mask == kb_Log) return FUNC_10_X;
            if (mask == kb_Square) return FUNC_X_SQUARED;
            if (mask == kb_Recip) return FUNC_X_INV;
            if (mask == kb_Math) return FUNC_TEST;
        }
        // Group 3: More 2nd functions
        else if (group == 3) {
            if (mask == kb_0) return FUNC_CATALOG;
            if (mask == kb_1) return FUNC_LINK;
            if (mask == kb_4) return FUNC_ANGLE;
            if (mask == kb_7) return FUNC_CALC_MENU;
            if (mask == kb_Comma) return '{';
            if (mask == kb_Sin) return FUNC_SIN_INV;
            if (mask == kb_Apps) return FUNC_MATRIX;
            if (mask == kb_GraphVar) return FUNC_DRAW;
        }
        // Group 4
        else if (group == 4) {
            if (mask == kb_DecPnt) return FUNC_ANS;
            if (mask == kb_2) return FUNC_UNIT;
            if (mask == kb_5) return FUNC_MEM;
            if (mask == kb_8) return FUNC_DISTR;
            if (mask == kb_LParen) return '{';
            if (mask == kb_Cos) return FUNC_COS_INV;
            if (mask == kb_Prgm) return FUNC_LIST;
            if (mask == kb_Stat) return FUNC_PROBABILITY;
        }
        // Group 5
        else if (group == 5) {
            if (mask == kb_Chs) return FUNC_ENTRY;
            if (mask == kb_3) return FUNC_SOLVE;
            if (mask == kb_6) return FUNC_PARAMETRIC;
            if (mask == kb_9) return FUNC_POLAR;
            if (mask == kb_RParen) return '}';
            if (mask == kb_Tan) return FUNC_TAN_INV;
            if (mask == kb_Vars) return FUNC_STRING;
        }
        // Special function keys
        else if (group == 6) {
            if (mask == kb_Enter) return KB_KEY_ENTER;
            if (mask == kb_Add) return FUNC_MEM_ADD;
            if (mask == kb_Sub) return FUNC_MEM_SUB;
            if (mask == kb_Mul) return FUNC_MEM_MUL;
            if (mask == kb_Div) return FUNC_MEM_DIV;
            if (mask == kb_Power) return FUNC_ROOT;
            if (mask == kb_Clear) return FUNC_RESET;
        }
    }

    // Default: return KB_KEY_NULL if no mapping found
    return KB_KEY_NULL;
}

/**
 * Gets a string representation of a key value (for debugging/display)
 */
void key_mapping_value_to_string(int key_value, char* buffer) {
    if (!buffer) return;

    // Handle special keys
    if (key_value < 32) {
        switch (key_value) {
            case KB_KEY_NULL: strcpy(buffer, "NULL"); break;
            case KB_KEY_ENTER: strcpy(buffer, "ENTER"); break;
            case KB_KEY_CLEAR: strcpy(buffer, "CLEAR"); break;
            case KB_KEY_DEL: strcpy(buffer, "DEL"); break;
            case KB_KEY_UP: strcpy(buffer, "UP"); break;
            case KB_KEY_DOWN: strcpy(buffer, "DOWN"); break;
            case KB_KEY_LEFT: strcpy(buffer, "LEFT"); break;
            case KB_KEY_RIGHT: strcpy(buffer, "RIGHT"); break;
            case KB_KEY_2ND: strcpy(buffer, "2ND"); break;
            case KB_KEY_ALPHA: strcpy(buffer, "ALPHA"); break;
            case KB_KEY_MODE: strcpy(buffer, "MODE"); break;
            default: sprintf(buffer, "CTRL-%d", key_value);
        }
    }
    // Handle normal ASCII characters
    else if (key_value >= 32 && key_value < 128) {
        // For printable ASCII, just use the character
        buffer[0] = (char)key_value;
        buffer[1] = '\0';
    }
    // Handle function keys
    else if (key_value >= 128) {
        switch (key_value) {
            case FUNC_Y_EQUALS: strcpy(buffer, "Y="); break;
            case FUNC_WINDOW: strcpy(buffer, "WINDOW"); break;
            case FUNC_ZOOM: strcpy(buffer, "ZOOM"); break;
            case FUNC_TRACE: strcpy(buffer, "TRACE"); break;
            case FUNC_GRAPH: strcpy(buffer, "GRAPH"); break;
            case FUNC_TABLE: strcpy(buffer, "TABLE"); break;
            case FUNC_FORMAT: strcpy(buffer, "FORMAT"); break;
            case FUNC_CALC: strcpy(buffer, "CALC"); break;
            case FUNC_MATRIX: strcpy(buffer, "MATRIX"); break;
            case FUNC_STAT: strcpy(buffer, "STAT"); break;
            case FUNC_MATH: strcpy(buffer, "MATH"); break;
            case FUNC_APPS: strcpy(buffer, "APPS"); break;
            case FUNC_PRGM: strcpy(buffer, "PRGM"); break;
            case FUNC_VARS: strcpy(buffer, "VARS"); break;
            case FUNC_SIN: strcpy(buffer, "SIN"); break;
            case FUNC_COS: strcpy(buffer, "COS"); break;
            case FUNC_TAN: strcpy(buffer, "TAN"); break;
            case FUNC_LOG: strcpy(buffer, "LOG"); break;
            case FUNC_LN: strcpy(buffer, "LN"); break;
            case FUNC_STO: strcpy(buffer, "STO→"); break;
            case FUNC_SQUARE: strcpy(buffer, "X²"); break;
            case FUNC_RECIP: strcpy(buffer, "1/X"); break;
            case FUNC_TRIG: strcpy(buffer, "TRIG"); break;
            case FUNC_SOLVER: strcpy(buffer, "SOLVER"); break;
            case FUNC_FRAC: strcpy(buffer, "FRAC"); break;
            case FUNC_X_VAR: strcpy(buffer, "X"); break;
            case FUNC_X_SQUARED: strcpy(buffer, "X²"); break;
            case FUNC_X_INV: strcpy(buffer, "X⁻¹"); break;
            case FUNC_EXP: strcpy(buffer, "e^X"); break;
            case FUNC_PI: strcpy(buffer, "π"); break;
            case FUNC_ANS: strcpy(buffer, "ANS"); break;
            case FUNC_ENTRY: strcpy(buffer, "ENTRY"); break;
            case FUNC_CATALOG: strcpy(buffer, "CATALOG"); break;
            case FUNC_SOLVE: strcpy(buffer, "SOLVE"); break;
            case FUNC_SIN_INV: strcpy(buffer, "SIN⁻¹"); break;
            case FUNC_COS_INV: strcpy(buffer, "COS⁻¹"); break;
            case FUNC_TAN_INV: strcpy(buffer, "TAN⁻¹"); break;
            case FUNC_10_X: strcpy(buffer, "10^X"); break;
            case FUNC_RECALL: strcpy(buffer, "RCL"); break;
            case FUNC_MEM_ADD: strcpy(buffer, "M+"); break;
            case FUNC_MEM_SUB: strcpy(buffer, "M-"); break;
            case FUNC_MEM_MUL: strcpy(buffer, "M×"); break;
            case FUNC_MEM_DIV: strcpy(buffer, "M÷"); break;
            case FUNC_ROOT: strcpy(buffer, "√"); break;
            case FUNC_RESET: strcpy(buffer, "RESET"); break;
            case FUNC_STATPLOT: strcpy(buffer, "STAT PLOT"); break;
            case FUNC_TBLSET: strcpy(buffer, "TBL SET"); break;
            case FUNC_QUIT: strcpy(buffer, "QUIT"); break;
            case FUNC_INS: strcpy(buffer, "INS"); break;
            case FUNC_TEST: strcpy(buffer, "TEST"); break;
            case FUNC_LINK: strcpy(buffer, "LINK"); break;
            case FUNC_ANGLE: strcpy(buffer, "ANGLE"); break;
            case FUNC_CALC_MENU: strcpy(buffer, "CALC MENU"); break;
            case FUNC_DRAW: strcpy(buffer, "DRAW"); break;
            case FUNC_UNIT: strcpy(buffer, "UNIT"); break;
            case FUNC_MEM: strcpy(buffer, "MEM"); break;
            case FUNC_DISTR: strcpy(buffer, "DISTR"); break;
            case FUNC_LIST: strcpy(buffer, "LIST"); break;
            case FUNC_PROBABILITY: strcpy(buffer, "PROB"); break;
            case FUNC_PARAMETRIC: strcpy(buffer, "PARAM"); break;
            case FUNC_POLAR: strcpy(buffer, "POLAR"); break;
            case FUNC_STRING: strcpy(buffer, "STRING"); break;
            default: sprintf(buffer, "FUNC-%d", key_value - 128);
        }
    }
    else {
        // Handle any other values
        sprintf(buffer, "KEY-%d", key_value);
    }
}
