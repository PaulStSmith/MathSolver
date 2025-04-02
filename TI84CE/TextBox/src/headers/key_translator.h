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

/**
 * Callback function type for key down events.
 * 
 * @param sender Pointer to the sender object.
 * @param value The logical value of the key pressed.
 */
typedef void (*CharDownCallback)(void* sender, int value);

/**
 * Callback function type for key press events.
 * 
 * @param sender Pointer to the sender object.
 * @param value The logical value of the key pressed.
 */
typedef void (*CharPressCallback)(void* sender, int value);

/**
 * Callback function type for key up events.
 * 
 * @param sender Pointer to the sender object.
 * @param value The logical value of the key released.
 */
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
 * Constants for special key values that don't have direct character representations.
 */
typedef enum {
    // Control keys (values below 32 to avoid ASCII overlap)
    CHAR_NULL      = 0,  /**< Null key (no action). */
    CHAR_ENTER     = 1,  /**< Enter key. */
    CHAR_CLEAR     = 2,  /**< Clear key. */
    CHAR_DEL       = 3,  /**< Delete key. */
    CHAR_UP        = 4,  /**< Up arrow key. */
    CHAR_DOWN      = 5,  /**< Down arrow key. */
    CHAR_LEFT      = 6,  /**< Left arrow key. */
    CHAR_RIGHT     = 7,  /**< Right arrow key. */
    CHAR_2ND       = 8,  /**< 2nd key (modifier). */
    CHAR_ALPHA     = 9,  /**< Alpha key (modifier). */
    CHAR_MODE      = 10, /**< Mode key. */
    CHAR_HOME      = 11, /**< Home key. */
    CHAR_END       = 12, /**< End key. */
    CHAR_INS       = 13, /**< Insert key. */
    CHAR_PGUP      = 14, /**< Page Up key. */
    CHAR_PGDN      = 15, /**< Page Down key. */
    
    // Function keys (values start at 128 to avoid any character conflicts)
    FUNC_Y_EQUALS    = 128, /**< Y= function key. */
    FUNC_WINDOW      = 129, /**< Window function key. */
    FUNC_ZOOM        = 130, /**< Zoom function key. */
    FUNC_TRACE       = 131, /**< Trace function key. */
    FUNC_GRAPH       = 132, /**< Graph function key. */
    FUNC_TABLE       = 133, /**< Table function key. */
    FUNC_FORMAT      = 134, /**< Format function key. */
    FUNC_CALC        = 135, /**< Calc function key. */
    FUNC_MATRIX      = 136, /**< Matrix function key. */
    FUNC_STAT        = 137, /**< Stat function key. */
    FUNC_MATH        = 138, /**< Math function key. */
    FUNC_APPS        = 139, /**< Apps function key. */
    FUNC_PRGM        = 140, /**< Program function key. */
    FUNC_VARS        = 141, /**< Vars function key. */
    FUNC_SIN         = 142, /**< Sin function key. */
    FUNC_COS         = 143, /**< Cos function key. */
    FUNC_TAN         = 144, /**< Tan function key. */
    FUNC_LOG         = 145, /**< Log function key. */
    FUNC_LN          = 146, /**< Ln function key. */
    FUNC_STO         = 147, /**< Sto function key. */
    FUNC_SQUARE      = 148, /**< Square function key. */
    FUNC_RECIP       = 149, /**< Reciprocal function key. */
    FUNC_TRIG        = 150, /**< Trig function key. */
    FUNC_SOLVER      = 151, /**< Solver function key. */
    FUNC_FRAC        = 152, /**< Fraction function key. */
    FUNC_X_VAR       = 153, /**< X variable function key. */
    FUNC_X_SQUARED   = 154, /**< X squared function key. */
    FUNC_X_INV       = 155, /**< X inverse function key. */
    FUNC_EXP         = 156, /**< Exponential function key. */
    FUNC_PI          = 157, /**< π constant (not a function key). */
    FUNC_E           = 190, /**< E constant (not a function key). */
    FUNC_ANS         = 158, /**< Ans function key. */
    FUNC_ENTRY       = 159, /**< Entry function key. */
    FUNC_CATALOG     = 160, /**< Catalog function key. */
    FUNC_SOLVE       = 161, /**< Solve function key. */
    
    // Inverse trig functions
    FUNC_SIN_INV     = 162, /**< Inverse Sin function key. */
    FUNC_COS_INV     = 163, /**< Inverse Cos function key. */
    FUNC_TAN_INV     = 164, /**< Inverse Tan function key. */
    
    // Other math functions
    FUNC_10_X        = 165, /**< 10^x function key. */
    FUNC_RECALL      = 166, /**< Recall function key. */
    FUNC_ROOT        = 167, /**< Root function key. */
    
    // Memory operations
    FUNC_MEM_ADD     = 168, /**< Memory add function key. */
    FUNC_MEM_SUB     = 169, /**< Memory subtract function key. */
    FUNC_MEM_MUL     = 170, /**< Memory multiply function key. */
    FUNC_MEM_DIV     = 171, /**< Memory divide function key. */
    
    // System functions
    FUNC_RESET       = 172, /**< Reset function key. */
    FUNC_QUIT        = 173, /**< Quit function key. */
    FUNC_INS         = 174, /**< Insert function key. */
    
    // Menu and category functions
    FUNC_STATPLOT    = 175, /**< Stat plot function key. */
    FUNC_TBLSET      = 176, /**< Table set function key. */
    FUNC_TEST        = 177, /**< Test function key. */
    FUNC_LINK        = 178, /**< Link function key. */
    FUNC_ANGLE       = 179, /**< Angle function key. */
    FUNC_CALC_MENU   = 180, /**< Calc menu function key. */
    FUNC_DRAW        = 181, /**< Draw function key. */
    FUNC_UNIT        = 182, /**< Unit function key. */
    FUNC_MEM         = 183, /**< Memory function key. */
    FUNC_DISTR       = 184, /**< Distribution function key. */
    FUNC_LIST        = 185, /**< List function key. */
    FUNC_PROBABILITY = 186, /**< Probability function key. */
    FUNC_PARAMETRIC  = 187, /**< Parametric function key. */
    FUNC_POLAR       = 188, /**< Polar function key. */
    FUNC_STRING      = 189  /**< String function key. */
} CharValue;

/**
 * Calculator keyboard modes.
 */
typedef enum {
    KB_MODE_NORMAL           = 0,  /**< Normal keyboard mode. */
    KB_MODE_2ND              = 1,  /**< 2nd modifier mode. */
    KB_MODE_ALPHA            = 2,  /**< Alpha modifier mode. */
    KB_MODE_LOWER            = 4,  /**< Lowercase alpha mode. */
    KB_MODE_LOCK             = 8,  /**< Locked mode. */
    KB_MODE_ALPHA_LOWER      = 6,  /**< Combined Alpha and Lowercase mode. */
    KB_MODE_ALPHA_LOCK       = 10, /**< Combined Alpha and Locked mode. */
    KB_MODE_ALPHA_LOWER_LOCK = 14  /**< Combined Alpha, Lowercase, and Locked mode. */
} KeyboardMode;

#include "key_translator_public.h"
#endif // KEY_TRANSLATOR_H
