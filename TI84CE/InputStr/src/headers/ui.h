/**
 * MathSolver for TI-84 CE - Text-based UI Definitions
 * 
 * This header file defines constants and function prototypes for the 
 * text-based user interface of the MathSolver application. It provides 
 * functions for displaying UI components, handling user input, and 
 * managing settings.
 */

#ifndef UI_H
#define UI_H

/* ============================== UI Constants ============================== */

/** Maximum number of rows that can be displayed on the screen. */
#define MAX_DISPLAY_ROWS     10

/** Maximum number of columns that can be displayed on the screen. */
#define MAX_DISPLAY_COLS     26

/** Maximum number of items that can be displayed in a menu. */
#define MAX_MENU_ITEMS       5

/** Number of frames between cursor blinks for visual feedback. */
#define CURSOR_BLINK_RATE    10

#include "ui_public.h"

#endif /* UI_H */
