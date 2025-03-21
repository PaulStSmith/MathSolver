#include "headers/log.h"
#include "headers/mathsolver.h"

int variable_count = 0;
int node_pool_index = 0;
Variable variables[MAX_VARIABLES];

/**
 * Sets a variable value.
 * 
 * @param name The name of the variable.
 * @param value The value to assign to the variable.
 */
void set_variable(const char* name, double value) {
    // Check if variable already exists
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            variables[i].is_defined = true;
            log_variable(name, value);
            return;
        }
    }
    
    // Add new variable if we have room
    if (variable_count < MAX_VARIABLES) {
        strncpy(variables[variable_count].name, name, MAX_TOKEN_LENGTH - 1);
        variables[variable_count].name[MAX_TOKEN_LENGTH - 1] = '\0';
        variables[variable_count].value = value;
        variables[variable_count].is_defined = true;
        variable_count++;
        log_variable(name, value);
    }
}

/**
 * Gets a variable value.
 * 
 * @param name The name of the variable.
 * @param found Pointer to a boolean that will be set to true if the variable is found, false otherwise.
 * @return The value of the variable, or 0 if not found.
 */
double get_variable(const char* name, bool* found) {
    // Check for built-in constants
    if (is_constant(name)) {
        *found = true;
        if (strcmp(name, "pi") == 0 || strcmp(name, "PI") == 0)
            return PI;
        else if (strcmp(name, "e") == 0 || strcmp(name, "E") == 0)
            return E;
        else if (strcmp(name, "phi") == 0 || strcmp(name, "PHI") == 0)
            return PHI;
    }
    
    // Special case: Check for the π character (ASCII 196)
    if (name[0] == (char)0xC4 && name[1] == '\0') {
        *found = true;
        return PI;
    }

    // Check user-defined variables
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0 && variables[i].is_defined) {
            *found = true;
            double value = variables[i].value;
            log_variable(name, value);
            return value;
        }
    }
    
    *found = false;
    log_error("Variable not found");
    return 0.0;
}

/**
 * Checks if a name is a mathematical constant.
 * 
 * @param name The name to check.
 * @return True if the name is a constant, false otherwise.
 */
bool is_constant(const char* name) {
    return (strcmp(name, "pi") == 0 || strcmp(name, "PI") == 0 || 
            strcmp(name, "e") == 0 || strcmp(name, "E") == 0 ||
            strcmp(name, "phi") == 0 || strcmp(name, "PHI") == 0);
}
