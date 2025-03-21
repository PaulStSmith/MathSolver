/**
 * MathSolver for TI-84 CE - Main Program
 * 
 * Main entry point for the calculator application
 */

 #include <tice.h>
 #include <graphx.h>
 #include "headers/mathsolver.h"
 #include "headers/ui.h"
 
 int main(void) {
     // Initialize the math solver
     mathsolver_init();
     
     // Set up some default variables for convenience
     set_variable("x", 0);
     set_variable("y", 0);
     
     // Set arithmetic mode (normal with 4 decimal places)
     set_arithmetic_mode(ARITHMETIC_NORMAL, 4, false);
     
     // Run the calculator UI
     run_calculator_ui();
     
     // Clean up
     mathsolver_cleanup();
     
     return 0;
 }
 