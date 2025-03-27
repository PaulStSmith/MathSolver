/**
 * MathSolver for TI-84 CE - Main Program
 * 
 * Main entry point for the calculator application.
 * This program initializes the math solver, sets up default variables,
 * configures arithmetic settings, and runs the calculator UI.
 */

#include <tice.h>
#include <graphx.h>
#include <ti/screen.h>
#include <keypadc.h>

#include "headers/ui.h"
#include "headers/log.h"
#include "headers/main_private.h"

/**
 * @brief Main function of the MathSolver application.
 * 
 * This function initializes the math solver, sets up default variables,
 * configures arithmetic settings, runs the calculator UI, and performs cleanup
 * before exiting.
 * 
 * @return int Returns 0 upon successful execution.
 */
int main(void) {

    logger_init();
    screen_init();
    
    // Clean up
    logger_close();
    
    return 0; // Exit the program successfully.
}

static void screen_init(void){
    log_debug("Initializing screen");
    os_ClrHome();
    kb_Reset();
    os_FontSelect(os_SmallFont);
    log_message("Font ID: %d", (int)os_FontGetID());
    uint24_t width = os_FontGetWidth("W");
    uint24_t height = os_FontGetHeight();
    log_message("Font size: (%d x %d)", width, height);
}
