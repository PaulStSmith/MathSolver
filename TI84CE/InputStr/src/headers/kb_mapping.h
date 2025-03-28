#ifndef KB_MAPPING_H
#define KB_MAPPING_H

#include <stdbool.h>
#include <tice.h>
#include <keypadc.h>
#include "kb_handler.h"

/**
 * Constants for special key values that don't have direct character representations
 */
typedef enum {
    // Control keys (values below 32 to avoid ASCII overlap)
    KB_KEY_NULL      = 0,
    KB_KEY_ENTER     = 1,
    KB_KEY_CLEAR     = 2,
    KB_KEY_DEL       = 3,
    KB_KEY_UP        = 4,
    KB_KEY_DOWN      = 5,
    KB_KEY_LEFT      = 6,
    KB_KEY_RIGHT     = 7,
    KB_KEY_2ND       = 8,
    KB_KEY_ALPHA     = 9,
    KB_KEY_MODE      = 10,
    
    // Function keys (values start at 128 to avoid any character conflicts)
    FUNC_Y_EQUALS    = 128,
    FUNC_WINDOW      = 129,
    FUNC_ZOOM        = 130,
    FUNC_TRACE       = 131,
    FUNC_GRAPH       = 132,
    FUNC_TABLE       = 133,
    FUNC_FORMAT      = 134,
    FUNC_CALC        = 135,
    FUNC_MATRIX      = 136,
    FUNC_STAT        = 137,
    FUNC_MATH        = 138,
    FUNC_APPS        = 139,
    FUNC_PRGM        = 140,
    FUNC_VARS        = 141,
    FUNC_SIN         = 142,
    FUNC_COS         = 143,
    FUNC_TAN         = 144,
    FUNC_LOG         = 145,
    FUNC_LN          = 146,
    FUNC_STO         = 147,
    FUNC_SQUARE      = 148,
    FUNC_RECIP       = 149,
    FUNC_TRIG        = 150,
    FUNC_SOLVER      = 151,
    FUNC_FRAC        = 152,
    FUNC_X_VAR       = 153,
    FUNC_X_SQUARED   = 154,
    FUNC_X_INV       = 155,
    FUNC_EXP         = 156,
    FUNC_PI          = 157,
    FUNC_ANS         = 158,
    FUNC_ENTRY       = 159,
    FUNC_CATALOG     = 160,
    FUNC_SOLVE       = 161,
    
    // Inverse trig functions
    FUNC_SIN_INV     = 162,
    FUNC_COS_INV     = 163,
    FUNC_TAN_INV     = 164,
    
    // Other math functions
    FUNC_10_X        = 165,
    FUNC_RECALL      = 166,
    FUNC_ROOT        = 167,
    
    // Memory operations
    FUNC_MEM_ADD     = 168,
    FUNC_MEM_SUB     = 169,
    FUNC_MEM_MUL     = 170,
    FUNC_MEM_DIV     = 171,
    
    // System functions
    FUNC_RESET       = 172,
    FUNC_QUIT        = 173,
    FUNC_INS         = 174,
    
    // Menu and category functions
    FUNC_STATPLOT    = 175,
    FUNC_TBLSET      = 176,
    FUNC_TEST        = 177,
    FUNC_LINK        = 178,
    FUNC_ANGLE       = 179,
    FUNC_CALC_MENU   = 180,
    FUNC_DRAW        = 181,
    FUNC_UNIT        = 182,
    FUNC_MEM         = 183,
    FUNC_DISTR       = 184,
    FUNC_LIST        = 185,
    FUNC_PROBABILITY = 186,
    FUNC_PARAMETRIC  = 187,
    FUNC_POLAR       = 188,
    FUNC_STRING      = 189
} SpecialKeyValue;

/**
 * Calculator keyboard modes
 */
typedef enum {
    KB_MODE_NORMAL           = 0,
    KB_MODE_ALPHA            = 1,
    KB_MODE_2ND              = 2,
    KB_MODE_ALPHA_LOCK       = 3,
    KB_MODE_ALPHA_LOCK_LOWER = 4 
} KeyboardMode;

/**
 * Structure to track keyboard mode state
 */
typedef struct {
    KeyboardMode current_mode;
    bool         alpha_active;
    bool         second_active;
    bool         alpha_lock;
} KeyboardState;

/**
 * Initialize the keyboard state tracker
 * 
 * @return The initialized keyboard state
 */
KeyboardState key_mapping_init(void);

/**
 * Process mode keys (2nd, Alpha) and update keyboard state
 * 
 * @param key The physical key that was pressed
 * @param state Pointer to the current keyboard state
 * @return True if the key was a mode key and processed, false otherwise
 */
bool key_mapping_process_mode_keys(CombinedKey key, KeyboardState* state);

/**
 * Gets the character or function identifier for a key based on current keyboard mode.
 * 
 * @param key The physical key that was pressed (CombinedKey format)
 * @param state The current keyboard state (mode)
 * @return The character, function code, or special value representing the key in current mode
 */
int key_mapping_get_value(CombinedKey key, KeyboardState state);

/**
 * Gets a string representation of a key value (for debugging/display)
 * 
 * @param KB_KEY_value The key value to convert to string
 * @param buffer The buffer to write the string to (must be at least 16 bytes)
 */
void key_mapping_value_to_string(int KB_KEY_value, char* buffer);

#endif /* KB_MAPPING_H */
