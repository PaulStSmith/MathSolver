/**
 * Combined key definitions for TI-84 CE
 * 
 * This header combines both the group (kb_Data index) and key mask
 * into a single value for easier key handling.
 * 
 * Format: Each KEY_* constant contains both the group and the key mask:
 * - Bits 0-7: Key mask within the group
 * - Bits 8-10: Group index (0-7)
 */

#ifndef TI84CE_KEYS_H
#define TI84CE_KEYS_H

#include <keypadc.h>

// Helper macros to extract group and mask from combined key value
#define KEY_GROUP(key) ((key) >> 8)
#define KEY_MASK(key)  ((key) & 0xFF)

// Helper macro to create a combined key value
#define MAKE_KEY(group, mask) (((group) << 8) | (mask))

// Combined key definitions
typedef enum {
    // No key pressed
    KEY_NONE     = MAKE_KEY(0, 0),

    // Group 1
    KEY_GRAPH    = MAKE_KEY(1, kb_Graph),
    KEY_TRACE    = MAKE_KEY(1, kb_Trace),
    KEY_ZOOM     = MAKE_KEY(1, kb_Zoom),
    KEY_WINDOW   = MAKE_KEY(1, kb_Window),
    KEY_YEQU     = MAKE_KEY(1, kb_Yequ),
    KEY_2ND      = MAKE_KEY(1, kb_2nd),
    KEY_MODE     = MAKE_KEY(1, kb_Mode),
    KEY_DEL      = MAKE_KEY(1, kb_Del),
    
    // Group 2
    KEY_STO      = MAKE_KEY(2, kb_Sto),
    KEY_LN       = MAKE_KEY(2, kb_Ln),
    KEY_LOG      = MAKE_KEY(2, kb_Log),
    KEY_SQUARE   = MAKE_KEY(2, kb_Square),
    KEY_RECIP    = MAKE_KEY(2, kb_Recip),
    KEY_MATH     = MAKE_KEY(2, kb_Math),
    KEY_ALPHA    = MAKE_KEY(2, kb_Alpha),
    
    // Group 3
    KEY_0        = MAKE_KEY(3, kb_0),
    KEY_1        = MAKE_KEY(3, kb_1),
    KEY_4        = MAKE_KEY(3, kb_4),
    KEY_7        = MAKE_KEY(3, kb_7),
    KEY_COMMA    = MAKE_KEY(3, kb_Comma),
    KEY_SIN      = MAKE_KEY(3, kb_Sin),
    KEY_APPS     = MAKE_KEY(3, kb_Apps),
    KEY_GRAPHVAR = MAKE_KEY(3, kb_GraphVar),
    
    // Group 4
    KEY_DECPNT   = MAKE_KEY(4, kb_DecPnt),
    KEY_2        = MAKE_KEY(4, kb_2),
    KEY_5        = MAKE_KEY(4, kb_5),
    KEY_8        = MAKE_KEY(4, kb_8),
    KEY_LPAREN   = MAKE_KEY(4, kb_LParen),
    KEY_COS      = MAKE_KEY(4, kb_Cos),
    KEY_PRGM     = MAKE_KEY(4, kb_Prgm),
    KEY_STAT     = MAKE_KEY(4, kb_Stat),
    
    // Group 5
    KEY_CHS      = MAKE_KEY(5, kb_Chs),
    KEY_3        = MAKE_KEY(5, kb_3),
    KEY_6        = MAKE_KEY(5, kb_6),
    KEY_9        = MAKE_KEY(5, kb_9),
    KEY_RPAREN   = MAKE_KEY(5, kb_RParen),
    KEY_TAN      = MAKE_KEY(5, kb_Tan),
    KEY_VARS     = MAKE_KEY(5, kb_Vars),
    
    // Group 6
    KEY_ENTER    = MAKE_KEY(6, kb_Enter),
    KEY_ADD      = MAKE_KEY(6, kb_Add),
    KEY_SUB      = MAKE_KEY(6, kb_Sub),
    KEY_MUL      = MAKE_KEY(6, kb_Mul),
    KEY_DIV      = MAKE_KEY(6, kb_Div),
    KEY_POWER    = MAKE_KEY(6, kb_Power),
    KEY_CLEAR    = MAKE_KEY(6, kb_Clear),
    
    // Group 7
    KEY_DOWN     = MAKE_KEY(7, kb_Down),
    KEY_LEFT     = MAKE_KEY(7, kb_Left),
    KEY_RIGHT    = MAKE_KEY(7, kb_Right),
    KEY_UP       = MAKE_KEY(7, kb_Up)
} CombinedKey;

#endif // TI84CE_KEYS_H
