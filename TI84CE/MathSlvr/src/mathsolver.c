/**
 * MathSolver for TI-84 CE - Core Implementation
 * 
 * This file implements the core functionality of the MathSolver, including
 * tokenization, parsing, expression evaluation, variable management, and
 * arithmetic formatting.
 */

#include "headers/log.h"
#include "headers/mathsolver_private.h"

/* ============================== Global Variables ============================== */

ArithmeticType current_arithmetic_type = ARITHMETIC_NORMAL;
int current_precision = 4;
bool current_use_significant_digits = false;

/**
 * Static memory pool for expression nodes.
 * Used to allocate nodes for the expression tree.
 */
static ExpressionNode node_pool[MAX_NODES];

real_t PI;
real_t E;
real_t PHI;
real_t ZERO;
real_t LOG10;

/**
 * Initializes the math solver.
 * Resets the node pool and variable list to their initial states.
 */
void mathsolver_init(void) {
    init_constants();

    // Reset node pool
    node_pool_index = 0;
    memset(node_pool, 0, sizeof(node_pool));
    
    // Reset variables
    variable_count = 0;
    memset(variables, 0, sizeof(variables));

    log_debug("MathSolver initialized");
}

/**
 * Initializes the math solver constants.
 * Sets up the values for PI, E, PHI, ZERO, and LOG10.
 */
static void init_constants(void)
{
    real_t x = os_FloatToReal(10.0);
    E = os_FloatToReal(2.71828182845904523536);
    PI = os_FloatToReal(3.14159265358979323846);
    PHI = os_FloatToReal(1.61803398874989484820);
    ZERO = os_Int24ToReal(0);
    LOG10 = os_RealLog(&x);
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
