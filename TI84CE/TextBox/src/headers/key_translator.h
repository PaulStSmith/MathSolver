/**
 * Key Translator for TI-84 CE (Layer 2)
 * 
 * Manages keyboard modes (Normal, 2nd, Alpha, etc.) and translates
 * physical key presses into logical values based on current mode.
 */

#ifndef KEY_TRANSLATOR_H
#define KEY_TRANSLATOR_H

#include <stdbool.h>
#include "keyboard.h"

// Callback types for character events
typedef void (*CharDownCallback)(void* sender, int value);
typedef void (*CharPressCallback)(void* sender, int value);
typedef void (*CharUpCallback)(void* sender, int value);

/** 
 * Internal structure for callback registration.
 * Stores information about a registered callback, including its type, ID, and function pointer.
 */
typedef struct {
    bool active;                     /**< Whether this entry is active. */
    int id;                          /**< Unique ID for this callback. */
    union {
        CharDownCallback down;       /**< Function pointer for down callback. */
        CharPressCallback press;     /**< Function pointer for press callback. */
        CharUpCallback up;           /**< Function pointer for up callback. */
    } callback;                      /**< Union to store the callback function pointer. */
    int type;                        /**< Type of callback (0=down, 1=press, 2=up). */
    int repeat_delay;                /**< Time before repeating (for press). */
    int repeat_interval;             /**< Interval between repeats (for press). */
} CharCallbackEntry;

/** Callback types for character events. */
enum {
    CHAR_CB_DOWN = 0,  /**< Callback for key down events. */
    CHAR_CB_PRESS = 1, /**< Callback for key press events. */
    CHAR_CB_UP = 2     /**< Callback for key up events. */
};

/**
 * Constants for special key values that don't have direct character representations
 */
typedef enum {
    // Control keys (values below 32 to avoid ASCII overlap)
    CHAR_NULL      = 0,
    CHAR_ENTER     = 1,
    CHAR_CLEAR     = 2,
    CHAR_DEL       = 3,
    CHAR_UP        = 4,
    CHAR_DOWN      = 5,
    CHAR_LEFT      = 6,
    CHAR_RIGHT     = 7,
    CHAR_2ND       = 8,
    CHAR_ALPHA     = 9,
    CHAR_MODE      = 10,
    CHAR_HOME      = 11,
    CHAR_END       = 12,
    CHAR_INS       = 13,
    CHAR_PGUP      = 14,
    CHAR_PGDN      = 15,
    
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
    FUNC_PI          = 157, // π constant (not a function key)
    FUNC_E           = 190, // E constant (not a function key)
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
} CharValue;

/**
 * Calculator keyboard modes
 */
typedef enum {
    KB_MODE_NORMAL           = 0,
    KB_MODE_2ND              = 1,
    KB_MODE_ALPHA            = 2,
    KB_MODE_LOWER            = 4,
    KB_MODE_LOCK             = 8,
    KB_MODE_ALPHA_LOWER      = 6,   // ALPHA | LOWER
    KB_MODE_ALPHA_LOCK       = 10,  // ALPHA | LOCKED
    KB_MODE_ALPHA_LOWER_LOCK = 14   // ALPHA | LOWER | LOCKED
} KeyboardMode;

#include "key_translator_public.h"
#endif // KEY_TRANSLATOR_H
