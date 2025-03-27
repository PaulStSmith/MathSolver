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

#include "headers/mathsolver.h"
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
    mathsolver_init();
    
    // Set up some default variables for convenience
    set_variable("x", ZERO); // Initializes variable 'x' with a default value of 0.
    set_variable("y", ZERO); // Initializes variable 'y' with a default value of 0.
    
    // Set arithmetic mode to normal.
    // Note: Setting up normal arithmetic the number of decimal places is ignored, as is the use of significant digits.
    set_arithmetic_mode(ARITHMETIC_NORMAL, 4, false); 
    
    // Run the calculator UI
    run_calculator_ui();
    
    // Clean up
    mathsolver_cleanup(); 
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
