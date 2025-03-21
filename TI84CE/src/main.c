/**
 * MathSolver for TI-84 CE - Main Program
 * 
 * Main entry point for the calculator application.
 * This program initializes the math solver, sets up default variables,
 * configures arithmetic settings, and runs the calculator UI.
 */

#include <tice.h>
#include <graphx.h>
#include "headers/mathsolver.h"
#include "headers/ui.h"

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
    // Initialize the math solver
    mathsolver_init(); // Sets up the math solver environment and resources.
    
    // Set up some default variables for convenience
    set_variable("x", 0); // Initializes variable 'x' with a default value of 0.
    set_variable("y", 0); // Initializes variable 'y' with a default value of 0.
    
    // Set arithmetic mode (normal with 4 decimal places)
    set_arithmetic_mode(ARITHMETIC_NORMAL, 4, false); 
    // Configures arithmetic mode to normal with 4 decimal places and no scientific notation.
    
    // Run the calculator UI
    run_calculator_ui(); // Launches the user interface for the calculator.
    
    // Clean up
    mathsolver_cleanup(); // Frees resources and performs cleanup operations.
    
    return 0; // Exit the program successfully.
}
