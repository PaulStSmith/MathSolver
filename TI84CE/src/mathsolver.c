/**
 * MathSolver for TI-84 CE - Core Implementation
 * 
 * This file implements the core functionality of the MathSolver, including
 * tokenization, parsing, expression evaluation, variable management, and
 * arithmetic formatting.
 */

#include "headers/log.h"
#include "headers/mathsolver.h"

/* ============================== Global Variables ============================== */

ArithmeticType current_arithmetic_type = ARITHMETIC_NORMAL;
int current_precision = 4;
bool current_use_significant_digits = false;

/**
 * Static memory pool for expression nodes.
 * Used to allocate nodes for the expression tree.
 */
static ExpressionNode node_pool[MAX_NODES];

/* ============================== Initialization and Cleanup ============================== */

/**
 * Initializes the math solver.
 * Resets the node pool and variable list to their initial states.
 */
void mathsolver_init(void) {
    // Reset node pool
    node_pool_index = 0;
    memset(node_pool, 0, sizeof(node_pool));
    
    // Reset variables
    variable_count = 0;
    memset(variables, 0, sizeof(variables));

    log_debug("MathSolver initialized");
}

/**
 * Cleans up the math solver resources.
 * Resets the node pool and variable list.
 */
void mathsolver_cleanup(void) {
    // Reset node pool
    node_pool_index = 0;
    
    // Reset variables
    variable_count = 0;

    log_debug("MathSolver cleaned up");
}
