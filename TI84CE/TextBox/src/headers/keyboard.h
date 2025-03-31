/**
 * Keyboard Handler for TI-84 CE (Layer 1)
 * 
 * Handles direct interaction with the calculator's hardware keyboard.
 * Provides physical key detection, debouncing, and callback registration.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <keypadc.h>

/**
 * Callback types enumeration.
 */
enum {
    CB_DOWN = 0, /**< Key down event */
    CB_PRESS = 1, /**< Key press event */
    CB_UP = 2    /**< Key up event */
};

/**
 * Combined key definition - contains both group and key mask
 * - Bits 0-7: Key mask within the group
 * - Bits 8-10: Group index (0-7)
 */
typedef enum {
    // No key pressed
    KEY_NONE     = 0,

    // Group 1
    KEY_GRAPH    = (1 << 8) | kb_Graph,
    KEY_TRACE    = (1 << 8) | kb_Trace,
    KEY_ZOOM     = (1 << 8) | kb_Zoom,
    KEY_WINDOW   = (1 << 8) | kb_Window,
    KEY_YEQU     = (1 << 8) | kb_Yequ,
    KEY_2ND      = (1 << 8) | kb_2nd,
    KEY_MODE     = (1 << 8) | kb_Mode,
    KEY_DEL      = (1 << 8) | kb_Del,
    
    // Group 2
    KEY_STO      = (2 << 8) | kb_Sto,
    KEY_LN       = (2 << 8) | kb_Ln,
    KEY_LOG      = (2 << 8) | kb_Log,
    KEY_SQUARE   = (2 << 8) | kb_Square,
    KEY_RECIP    = (2 << 8) | kb_Recip,
    KEY_MATH     = (2 << 8) | kb_Math,
    KEY_ALPHA    = (2 << 8) | kb_Alpha,
    
    // Group 3
    KEY_0        = (3 << 8) | kb_0,
    KEY_1        = (3 << 8) | kb_1,
    KEY_4        = (3 << 8) | kb_4,
    KEY_7        = (3 << 8) | kb_7,
    KEY_COMMA    = (3 << 8) | kb_Comma,
    KEY_SIN      = (3 << 8) | kb_Sin,
    KEY_APPS     = (3 << 8) | kb_Apps,
    KEY_GRAPHVAR = (3 << 8) | kb_GraphVar,
    
    // Group 4
    KEY_DECPNT   = (4 << 8) | kb_DecPnt,
    KEY_2        = (4 << 8) | kb_2,
    KEY_5        = (4 << 8) | kb_5,
    KEY_8        = (4 << 8) | kb_8,
    KEY_LPAREN   = (4 << 8) | kb_LParen,
    KEY_COS      = (4 << 8) | kb_Cos,
    KEY_PRGM     = (4 << 8) | kb_Prgm,
    KEY_STAT     = (4 << 8) | kb_Stat,
    
    // Group 5
    KEY_CHS      = (5 << 8) | kb_Chs,
    KEY_3        = (5 << 8) | kb_3,
    KEY_6        = (5 << 8) | kb_6,
    KEY_9        = (5 << 8) | kb_9,
    KEY_RPAREN   = (5 << 8) | kb_RParen,
    KEY_TAN      = (5 << 8) | kb_Tan,
    KEY_VARS     = (5 << 8) | kb_Vars,
    
    // Group 6
    KEY_ENTER    = (6 << 8) | kb_Enter,
    KEY_ADD      = (6 << 8) | kb_Add,
    KEY_SUB      = (6 << 8) | kb_Sub,
    KEY_MUL      = (6 << 8) | kb_Mul,
    KEY_DIV      = (6 << 8) | kb_Div,
    KEY_POWER    = (6 << 8) | kb_Power,
    KEY_CLEAR    = (6 << 8) | kb_Clear,
    
    // Group 7
    KEY_DOWN     = (7 << 8) | kb_Down,
    KEY_LEFT     = (7 << 8) | kb_Left,
    KEY_RIGHT    = (7 << 8) | kb_Right,
    KEY_UP       = (7 << 8) | kb_Up
} Key;

// Helper macros to extract group and mask from combined key value
#define KEY_GROUP(key) (((key) >> 8) & 0x7)
#define KEY_MASK(key)  ((key) & 0xFF)

// Callback types for key events
typedef void (*KeyDownCallback)(void* sender, Key key);
typedef void (*KeyPressCallback)(void* sender, Key key);
typedef void (*KeyUpCallback)(void* sender, Key key);
typedef void (*KeyHoldCallback)(void* sender, Key key, int hold_time);

/**
 * Internal structure for callback registration.
 */
typedef struct {
    bool active;                 /**< Whether this entry is active */
    int id;                      /**< Unique ID for this callback */
    union {
        KeyDownCallback down;    /**< Function pointer for down callback */
        KeyPressCallback press;  /**< Function pointer for press callback */
        KeyUpCallback up;        /**< Function pointer for up callback */
    } callback;
    void* obj;                   /**< Pointer to the sender of the callback */
    int type;                    /**< Type of callback (0=down, 1=press, 2=up) */
    int repeat_delay;            /**< Time before repeating (for press) */
    int repeat_interval;         /**< Interval between repeats (for press) */
} CallbackEntry;

#include "keyboard_public.h"

#endif // KEYBOARD_H
