#include <string.h>
#include <stdio.h>
#include "headers/kb_mapping_private.h"

/**
 * Initialize the keyboard state tracker.
 * 
 * @return A KeyboardState struct with default values.
 */
KeyboardState key_mapping_init(void) {
    KeyboardState state;
    memset(&state, 0, sizeof(KeyboardState));
    setKeyboardState(&state, KB_MODE_NORMAL);
    return state;
}

/**
 * Set the keyboard mode and update the state accordingly.
 * 
 * @param state Pointer to the current keyboard state.
 * @param mode The new keyboard mode to set.
 */
void setKeyboardState(KeyboardState* state, KeyboardMode mode) {
    if (!state) return;    
    state->current_mode = mode;
    
    // Keep legacy booleans in sync
    state->is_alpha = (mode & KB_MODE_ALPHA) != 0;
    state->is_second = (mode & KB_MODE_2ND) != 0;
    state->is_alpha_lower = (mode & KB_MODE_LOWER) != 0;
    state->is_alpha_lock = (mode & KB_MODE_ALPHA_LOCK) != 0;
    state->is_alpha_lower_lock = (mode & KB_MODE_ALPHA_LOWER_LOCK) != 0;
}

/**
 * Process mode keys (2nd, Alpha) and update keyboard state.
 * 
 * @param key The key being processed.
 * @param state Pointer to the current keyboard state.
 * @return True if the key was a mode key and processed, otherwise false.
 */
bool key_mapping_process_mode_keys(CombinedKey key, KeyboardState* state) {
    if (!state) return false;

    bool result = false;
    KeyboardMode mode = state->current_mode;

    // Check for mode keys
    if (key == KB_KEY_2ND) {
        // 2nd key pressed - toggle 2nd mode
        mode ^= KB_MODE_2ND;
        result = true;
    } else if (key == KEY_ALPHA) {
        // Alpha key pressed
        if (mode & ~KB_MODE_ALPHA) {
            mode |= KB_MODE_ALPHA;              // Set alpha mode
        } else if (mode & KB_MODE_ALPHA) {
            // If we are in alpha mode, change to alpha lower
            mode |= KB_MODE_ALPHA_LOWER;        // Set alpha lower mode
        } else if (mode & KB_MODE_LOWER) {
            // If we are in lower mode, return to normal mode
            mode = mode & ~KB_MODE_ALPHA_LOWER; // Remove alpha lower mode
            mode = mode & ~KB_MODE_LOCK;      // Remove locked mode
        }
        
        if (mode & KB_MODE_2ND) {
            mode |= KB_MODE_LOCK;             // Set locked mode
        } 
        mode = mode & ~KB_MODE_2ND;             // Remove 2nd mode
        result = true;
    }

    // Update the keyboard state
    setKeyboardState(state, mode); 
    return result;
}

/**
 * Gets the character or function identifier for a key based on the current keyboard mode.
 * 
 * @param key The key being processed.
 * @param state The current keyboard state.
 * @return The character or function identifier for the key.
 */
int key_mapping_get_value(CombinedKey key, KeyboardState state) {
    int group     = KEY_GROUP(key);
    int mask      = KEY_MASK(key);
    bool is_2nd   = state.is_second;
    bool is_alpha = state.is_alpha;
    bool is_lower = state.is_alpha_lower;
    bool is_normal = (!is_2nd && !is_alpha && !is_lower);

    // Handle normal mode (no alpha, no 2nd)
    if (is_normal) {
        // Group 1: Graph, Trace, Zoom, Window, Y=, 2nd, Mode, Del
        if (group == 1) {
            if (mask == kb_Graph)    return FUNC_GRAPH;
            if (mask == kb_Trace)    return FUNC_TRACE;
            if (mask == kb_Zoom)     return FUNC_ZOOM;
            if (mask == kb_Window)   return FUNC_WINDOW;
            if (mask == kb_Yequ)     return FUNC_Y_EQUALS;
            if (mask == kb_2nd)      return KB_KEY_2ND;     // Should be processed by key_mapping_process_mode_keys
            if (mask == kb_Mode)     return KB_KEY_MODE;
            if (mask == kb_Del)      return KB_KEY_DEL;
        }
        // Group 2: Sto, Ln, Log, x², 1/x, Math, Alpha
        else if (group == 2) {
            if (mask == kb_Sto)      return FUNC_STO;
            if (mask == kb_Ln)       return FUNC_LN;
            if (mask == kb_Log)      return FUNC_LOG;
            if (mask == kb_Square)   return FUNC_SQUARE;
            if (mask == kb_Recip)    return FUNC_RECIP;
            if (mask == kb_Math)     return FUNC_MATH;
            if (mask == kb_Alpha)    return KB_KEY_ALPHA; // Should be processed by key_mapping_process_mode_keys
        }
        // Group 3: 0, 1, 4, 7, ,, sin, apps, x
        else if (group == 3) {
            if (mask == kb_0)        return '0';
            if (mask == kb_1)        return '1';
            if (mask == kb_4)        return '4';
            if (mask == kb_7)        return '7';
            if (mask == kb_Comma)    return ',';
            if (mask == kb_Sin)      return FUNC_SIN;
            if (mask == kb_Apps)     return FUNC_APPS;
            if (mask == kb_GraphVar) return FUNC_X_VAR;
        }
        // Group 4: ., 2, 5, 8, (, cos, prgm, stat
        else if (group == 4) {
            if (mask == kb_DecPnt)  return '.';
            if (mask == kb_2)       return '2';
            if (mask == kb_5)       return '5';
            if (mask == kb_8)       return '8';
            if (mask == kb_LParen)  return '(';
            if (mask == kb_Cos)     return FUNC_COS;
            if (mask == kb_Prgm)    return FUNC_PRGM;
            if (mask == kb_Stat)    return FUNC_STAT;
        }
        // Group 5: (-), 3, 6, 9, ), tan, vars
        else if (group == 5) {
            if (mask == kb_Chs)      return FUNC_PI; // This is a bit unusual, but often (-) is used for π in normal mode
            if (mask == kb_3)        return '3';
            if (mask == kb_6)        return '6';
            if (mask == kb_9)        return '9';
            if (mask == kb_RParen)   return ')';
            if (mask == kb_Tan)      return FUNC_TAN;
            if (mask == kb_Vars)     return FUNC_VARS;
        }
        // Group 6: Enter, +, -, *, /, ^, clear
        else if (group == 6) {
            if (mask == kb_Enter)    return KB_KEY_ENTER;
            if (mask == kb_Add)      return '+';
            if (mask == kb_Sub)      return '-';
            if (mask == kb_Mul)      return '*';
            if (mask == kb_Div)      return '/';
            if (mask == kb_Power)    return '^';
            if (mask == kb_Clear)    return KB_KEY_CLEAR;
        }
        // Group 7: down, left, right, up
        else if (group == 7) {
            if (mask == kb_Down)     return KB_KEY_DOWN;
            if (mask == kb_Left)     return KB_KEY_LEFT;
            if (mask == kb_Right)    return KB_KEY_RIGHT;
            if (mask == kb_Up)       return KB_KEY_UP;
        }
    }
    // Handle alpha modes (both regular and lowercase)
    else if (is_alpha && !is_2nd) {
        // Per the Keyboard Modes.md file
        if (group == 2) {
            if (mask == kb_Math)     return is_lower ? 'a' : 'A';
            if (mask == kb_Recip)    return is_lower ? 'd' : 'D';
            if (mask == kb_Square)   return is_lower ? 'i' : 'I';
            if (mask == kb_Log)      return is_lower ? 'n' : 'N';
            if (mask == kb_Ln)       return is_lower ? 's' : 'S';
            if (mask == kb_Sto)      return is_lower ? 'x' : 'X';
        }
        else if (group == 3) {
            if (mask == kb_Apps)     return is_lower ? 'b' : 'B';
            if (mask == kb_Sin)      return is_lower ? 'e' : 'E';
            if (mask == kb_7)        return is_lower ? 'o' : 'O';
            if (mask == kb_4)        return is_lower ? 't' : 'T';
            if (mask == kb_1)        return is_lower ? 'y' : 'Y';
            if (mask == kb_0)        return ' '; // Space
            if (mask == kb_Comma)    return is_lower ? 'j' : 'J';
        }
        else if (group == 4) {
            if (mask == kb_Prgm)     return is_lower ? 'c' : 'C';
            if (mask == kb_Cos)      return is_lower ? 'f' : 'F';
            if (mask == kb_8)        return is_lower ? 'p' : 'P';
            if (mask == kb_5)        return is_lower ? 'u' : 'U';
            if (mask == kb_2)        return is_lower ? 'z' : 'Z';
            if (mask == kb_DecPnt)   return ':';
            if (mask == kb_LParen)   return is_lower ? 'k' : 'K';
        }
        else if (group == 5) {
            if (mask == kb_Tan)      return is_lower ? 'g' : 'G';
            if (mask == kb_6)        return is_lower ? 'v' : 'V';
            if (mask == kb_3)        return '\x5b'; // No lowercase for special chars
            if (mask == kb_Chs)      return '?';
            if (mask == kb_9)        return is_lower ? 'q' : 'Q';
            if (mask == kb_RParen)   return is_lower ? 'l' : 'L';
        }
        else if (group == 6) {
            if (mask == kb_Power)    return is_lower ? 'h' : 'H';
            if (mask == kb_Div)      return is_lower ? 'm' : 'M';
            if (mask == kb_Mul)      return is_lower ? 'r' : 'R';
            if (mask == kb_Sub)      return is_lower ? 'w' : 'W';
            if (mask == kb_Add)      return '"';
            // Keep other keys the same
            if (mask == kb_Enter)    return KB_KEY_ENTER;
            if (mask == kb_Clear)    return KB_KEY_CLEAR;
        }
        // Arrow keys (Group 7) - keep the same in all modes
        else if (group == 7) {
            if (mask == kb_Down)     return KB_KEY_DOWN;
            if (mask == kb_Left)     return KB_KEY_LEFT;
            if (mask == kb_Right)    return KB_KEY_RIGHT;
            if (mask == kb_Up)       return KB_KEY_UP;
        }
    }
    // Handle 2nd mode
    else if (is_2nd) {
        // Per the Keyboard Modes.md file
        if (group == 2) {
            if (mask == kb_Recip)    return FUNC_X_INV;     // ^-1
            if (mask == kb_Square)   return FUNC_ROOT;      // sqrt(
            if (mask == kb_Log)      return FUNC_10_X;      // 10^
            if (mask == kb_Ln)       return FUNC_EXP;       // e^x
            if (mask == kb_Sto)     return FUNC_RECALL;
            if (mask == kb_Math)    return FUNC_TEST;
        }
        else if (group == 3) {
            if (mask == kb_Sin)      return FUNC_SIN_INV;   // asin(
            if (mask == kb_7)        return 'u';
            if (mask == kb_1)        return 0;              // Empty or special value?
            if (mask == kb_0)       return FUNC_CATALOG;
            if (mask == kb_Apps)    return FUNC_MATRIX;
            if (mask == kb_GraphVar) return FUNC_DRAW;
            if (mask == kb_4)       return FUNC_ANGLE;
        }
        else if (group == 4) {
            if (mask == kb_Cos)      return FUNC_COS_INV;   // acos(
            if (mask == kb_8)        return 'v';
            if (mask == kb_2)        return 0;              // Empty or special value?
            if (mask == kb_LParen)   return '{';
            if (mask == kb_DecPnt)   return 0xD7;           // Mathematical constant i
            if (mask == kb_Prgm)    return FUNC_LIST;
            if (mask == kb_Stat)    return FUNC_PROBABILITY;
            if (mask == kb_5)       return FUNC_MEM;
        }
        else if (group == 5) {
            if (mask == kb_Tan)      return FUNC_TAN_INV;   // atan(
            if (mask == kb_9)        return 'w';
            if (mask == kb_RParen)   return '}';
            if (mask == kb_Chs)     return FUNC_ENTRY;
            if (mask == kb_Vars)    return FUNC_STRING;
            if (mask == kb_3)       return FUNC_SOLVE;
            if (mask == kb_6)       return FUNC_PARAMETRIC;
        }
        else if (group == 6) {
            if (mask == kb_Power)    return 0xC4;           // Mathematical constant pi
            if (mask == kb_Div)      return 0xDB;           // Constant e
            if (mask == kb_Mul)      return 0x5B;           // '[' on TI84
            if (mask == kb_Sub)      return ']';
            if (mask == kb_Enter)   return KB_KEY_ENTER;
            if (mask == kb_Add)     return FUNC_MEM_ADD;
            if (mask == kb_Clear)   return FUNC_RESET;
        }
        else if (group == 7) {
            if (mask == kb_Up)      return KB_KEY_PGUP;
            if (mask == kb_Down)    return KB_KEY_PGDN;
            if (mask == kb_Left)    return KB_KEY_HOME;
            if (mask == kb_Right)   return KB_KEY_END;
        }
    }

    // Default: return KB_KEY_NULL if no mapping found
    return KB_KEY_NULL;
}

/**
 * Gets a string representation of a key value (for debugging/display).
 * 
 * @param key_value The key value to convert.
 * @param buffer The buffer to store the string representation.
 */
void key_mapping_value_to_string(int key_value, char* buffer) {
    if (!buffer) return;

    // Handle special keys
    if (key_value < 32) {
        switch (key_value) {
            case KB_KEY_HOME:   strcpy(buffer, "HOME");  break;
            case KB_KEY_END:    strcpy(buffer, "END");   break;
            case KB_KEY_PGUP:   strcpy(buffer, "PGUP");  break;
            case KB_KEY_PGDN:   strcpy(buffer, "PGDN");  break;
            case KB_KEY_NULL:   strcpy(buffer, "NULL");  break;
            case KB_KEY_ENTER:  strcpy(buffer, "ENTER"); break;
            case KB_KEY_CLEAR:  strcpy(buffer, "CLEAR"); break;
            case KB_KEY_DEL:    strcpy(buffer, "DEL");   break;
            case KB_KEY_UP:     strcpy(buffer, "UP");    break;
            case KB_KEY_DOWN:   strcpy(buffer, "DOWN");  break;
            case KB_KEY_LEFT:   strcpy(buffer, "LEFT");  break;
            case KB_KEY_RIGHT:  strcpy(buffer, "RIGHT"); break;
            case KB_KEY_2ND:    strcpy(buffer, "2ND");   break;
            case KB_KEY_ALPHA:  strcpy(buffer, "ALPHA"); break;
            case KB_KEY_MODE:   strcpy(buffer, "MODE");  break;
            default:            sprintf(buffer, "CTRL-%d", key_value);
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
            case FUNC_Y_EQUALS:     strcpy(buffer, "Y=");       break;
            case FUNC_WINDOW:       strcpy(buffer, "WINDOW");   break;
            case FUNC_ZOOM:         strcpy(buffer, "ZOOM");     break;
            case FUNC_TRACE:        strcpy(buffer, "TRACE");    break;
            case FUNC_GRAPH:        strcpy(buffer, "GRAPH");    break;
            case FUNC_TABLE:        strcpy(buffer, "TABLE");    break;
            case FUNC_FORMAT:       strcpy(buffer, "FORMAT");   break;
            case FUNC_CALC:         strcpy(buffer, "CALC");     break;
            case FUNC_MATRIX:       strcpy(buffer, "MATRIX");   break;
            case FUNC_STAT:         strcpy(buffer, "STAT");     break;
            case FUNC_MATH:         strcpy(buffer, "MATH");     break;
            case FUNC_APPS:         strcpy(buffer, "APPS");     break;
            case FUNC_PRGM:         strcpy(buffer, "PRGM");     break;
            case FUNC_VARS:         strcpy(buffer, "VARS");     break;
            case FUNC_SIN:          strcpy(buffer, "SIN");      break;
            case FUNC_COS:          strcpy(buffer, "COS");      break;
            case FUNC_TAN:          strcpy(buffer, "TAN");      break;
            case FUNC_LOG:          strcpy(buffer, "LOG");      break;
            case FUNC_LN:           strcpy(buffer, "LN");       break;
            case FUNC_STO:          strcpy(buffer, "STO→");     break;
            case FUNC_SQUARE:       strcpy(buffer, "X²");       break;
            case FUNC_RECIP:        strcpy(buffer, "1/X");      break;
            case FUNC_TRIG:         strcpy(buffer, "TRIG");     break;
            case FUNC_SOLVER:       strcpy(buffer, "SOLVER");   break;
            case FUNC_FRAC:         strcpy(buffer, "FRAC");     break;
            case FUNC_X_VAR:        strcpy(buffer, "X");        break;
            case FUNC_X_SQUARED:    strcpy(buffer, "X²");       break;
            case FUNC_X_INV:        strcpy(buffer, "X⁻¹");      break;
            case FUNC_EXP:          strcpy(buffer, "e^X");      break;
            case FUNC_PI:           strcpy(buffer, "π");        break;
            case FUNC_ANS:          strcpy(buffer, "ANS");      break;
            case FUNC_ENTRY:        strcpy(buffer, "ENTRY");    break;
            case FUNC_CATALOG:      strcpy(buffer, "CATALOG");  break;
            case FUNC_SOLVE:        strcpy(buffer, "SOLVE");    break;
            case FUNC_SIN_INV:      strcpy(buffer, "SIN⁻¹");    break;
            case FUNC_COS_INV:      strcpy(buffer, "COS⁻¹");    break;
            case FUNC_TAN_INV:      strcpy(buffer, "TAN⁻¹");    break;
            case FUNC_10_X:         strcpy(buffer, "10^X");     break;
            case FUNC_RECALL:       strcpy(buffer, "RCL");      break;
            case FUNC_MEM_ADD:      strcpy(buffer, "M+");       break;
            case FUNC_MEM_SUB:      strcpy(buffer, "M-");       break;
            case FUNC_MEM_MUL:      strcpy(buffer, "M\xc3\x97"); break;
            case FUNC_MEM_DIV:      strcpy(buffer, "M÷");       break;
            case FUNC_ROOT:         strcpy(buffer, "√");        break;
            case FUNC_RESET:        strcpy(buffer, "RESET");    break;
            case FUNC_STATPLOT:     strcpy(buffer, "STAT PLOT"); break;
            case FUNC_TBLSET:       strcpy(buffer, "TBL SET");  break;
            case FUNC_QUIT:         strcpy(buffer, "QUIT");     break;
            case FUNC_INS:          strcpy(buffer, "INS");      break;
            case FUNC_TEST:         strcpy(buffer, "TEST");     break;
            case FUNC_LINK:         strcpy(buffer, "LINK");     break;
            case FUNC_ANGLE:        strcpy(buffer, "ANGLE");    break;
            case FUNC_CALC_MENU:    strcpy(buffer, "CALC MENU"); break;
            case FUNC_DRAW:         strcpy(buffer, "DRAW");     break;
            case FUNC_UNIT:         strcpy(buffer, "UNIT");     break;
            case FUNC_MEM:          strcpy(buffer, "MEM");      break;
            case FUNC_DISTR:        strcpy(buffer, "DISTR");    break;
            case FUNC_LIST:         strcpy(buffer, "LIST");     break;
            case FUNC_PROBABILITY:  strcpy(buffer, "PROB");     break;
            case FUNC_PARAMETRIC:   strcpy(buffer, "PARAM");    break;
            case FUNC_POLAR:        strcpy(buffer, "POLAR");    break;
            case FUNC_STRING:       strcpy(buffer, "STRING");   break;
            default:                sprintf(buffer, "FUNC-%d", key_value - 128);
        }
    }
    else {
        // Handle any other values
        sprintf(buffer, "KEY-%d", key_value);
    }
}
