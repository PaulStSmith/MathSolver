/**
 * MathSolver for TI-84 CE - Core Implementation
 * 
 * This file implements the core functionality of the MathSolver, including
 * tokenization, parsing, expression evaluation, variable management, and
 * arithmetic formatting.
 */

#include <tice.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "headers/log.h"
#include "headers/mathsolver.h"
#include "headers/mathsolver_private.h"

/* ============================== Global Variables ============================== */

/**
 * Static memory pool for expression nodes.
 * Used to allocate nodes for the expression tree.
 */
static ExpressionNode node_pool[MAX_NODES];

/** Index of the next available node in the node pool. */
static int node_pool_index = 0;

/**
 * Array of variables used in expressions.
 * Stores user-defined variables and their values.
 */
static Variable variables[MAX_VARIABLES];

/** Number of currently defined variables. */
static int variable_count = 0;

/**
 * Current arithmetic settings for calculations.
 * Includes arithmetic type, precision, and significant digit usage.
 */
static ArithmeticType current_arithmetic_type = ARITHMETIC_NORMAL;
static int current_precision = 4;
static bool current_use_significant_digits = false;

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

/* ============================== Tokenization Implementation ============================== */

/**
 * Advances to the next character in the input string.
 * Updates the tokenizer's position, line, and column.
 */
static void advance_position(Tokenizer* tokenizer) {
    if (tokenizer->input[tokenizer->position] == '\n') {
        tokenizer->line++;
        tokenizer->column = 1;
    } else {
        tokenizer->column++;
    }
    tokenizer->position++;
}

/**
 * Skips whitespace characters in the input string.
 * Advances the tokenizer's position past spaces, tabs, and newlines.
 */
static void skip_whitespace(Tokenizer* tokenizer) {
    while (tokenizer->input[tokenizer->position] != '\0' && 
           (tokenizer->input[tokenizer->position] == ' ' || 
            tokenizer->input[tokenizer->position] == '\t' || 
            tokenizer->input[tokenizer->position] == '\n' || 
            tokenizer->input[tokenizer->position] == '\r')) {
        advance_position(tokenizer);
    }
}

/**
 * Initializes a tokenizer with the given input string.
 * Prepares the tokenizer to start tokenizing the input.
 * 
 * @param tokenizer Pointer to the tokenizer to initialize.
 * @param input The input string to tokenize.
 */
void tokenizer_init(Tokenizer* tokenizer, const char* input) {
    tokenizer->input = input;
    tokenizer->position = 0;
    tokenizer->line = 1;
    tokenizer->column = 1;

    log_debug("Tokenizer initialized");
    log_message("Input string: %s", input);
    
    // Initialize with the first token
    tokenizer->current_token = get_next_token(tokenizer);
}

/**
 * Gets the next token from the input string.
 * Identifies numbers, variables, functions, and operators.
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return The next token in the input string.
 */
Token get_next_token(Tokenizer* tokenizer) {
    // Skip whitespace
    skip_whitespace(tokenizer);
    
    // Check if we're at the end of the input
    if (tokenizer->input[tokenizer->position] == '\0') {
        Token token;
        token.type = TOKEN_END;
        token.value[0] = '\0';
        token.position.start = tokenizer->position;
        token.position.end = tokenizer->position;
        token.position.line = tokenizer->line;
        token.position.column = tokenizer->column;

        log_debug("End of input reached");
        return token;
    }
    
    char current = tokenizer->input[tokenizer->position];
    Token token;
    token.position.start = tokenizer->position;
    token.position.line = tokenizer->line;
    token.position.column = tokenizer->column;
    
    // Check for numbers
    if ((current >= '0' && current <= '9') || current == '.') {
        int i = 0;
        bool has_decimal = false;
        
        // Collect all digits and decimal point
        while ((tokenizer->input[tokenizer->position] >= '0' && 
                tokenizer->input[tokenizer->position] <= '9') || 
               (tokenizer->input[tokenizer->position] == '.' && !has_decimal)) {
            
            if (tokenizer->input[tokenizer->position] == '.') {
                has_decimal = true;
            }
            
            token.value[i++] = tokenizer->input[tokenizer->position];
            advance_position(tokenizer);
            
            // Prevent buffer overflow
            if (i >= MAX_TOKEN_LENGTH - 1) break;
        }
        
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        token.position.end = tokenizer->position - 1;
        log_token("num.", token.type, token.value);
        return token;
    }
    
    // Check for variables and function names
    if ((current >= 'a' && current <= 'z') || 
        (current >= 'A' && current <= 'Z') ||
        current == '_') {
        
        int i = 0;
        
        // Collect all letters, digits, and underscores
        while ((tokenizer->input[tokenizer->position] >= 'a' && 
                tokenizer->input[tokenizer->position] <= 'z') || 
               (tokenizer->input[tokenizer->position] >= 'A' && 
                tokenizer->input[tokenizer->position] <= 'Z') ||
               (tokenizer->input[tokenizer->position] >= '0' && 
                tokenizer->input[tokenizer->position] <= '9') ||
               tokenizer->input[tokenizer->position] == '_') {

            /* lowercase the character */
            char temp = tokenizer->input[tokenizer->position];
            temp = (temp >= 'A' && temp <= 'Z') ? temp + 32 : temp;
            token.value[i++] = temp;

            advance_position(tokenizer);
            
            // Prevent buffer overflow
            if (i >= MAX_TOKEN_LENGTH - 1) break;
        }
        
        token.value[i] = '\0';
        
        // Check if this is a mathematical constant or function
        if (strcmp(token.value, "pi") == 0) {
            token.type = TOKEN_PI;
        } else if (strcmp(token.value, "phi") == 0) {
            token.type = TOKEN_PHI;
        } else if (strcmp(token.value, "sin") == 0 ||
            strcmp(token.value, "cos") == 0 ||
            strcmp(token.value, "tan") == 0 ||
            strcmp(token.value, "log") == 0 ||
            strcmp(token.value, "ln") == 0 ||
            strcmp(token.value, "sqrt") == 0) {
            token.type = TOKEN_FUNCTION;
        } else {
            // if not consider it a variable
            token.type = TOKEN_VARIABLE;
        }
        
        token.position.end = tokenizer->position - 1;
        log_token("func", token.type, token.value);
        return token;
    }
    
    // Check for single-character tokens
    token.value[0] = current;
    token.value[1] = '\0';
    advance_position(tokenizer);
    token.position.end = tokenizer->position - 1;
    
    switch (current) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_MULTIPLY; break;
        case '/': token.type = TOKEN_DIVIDE; break;
        case '^': token.type = TOKEN_POWER; break;
        case '(': token.type = TOKEN_LEFT_PAREN; break;
        case ')': token.type = TOKEN_RIGHT_PAREN; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '!': token.type = TOKEN_FACTORIAL; break;
        case (char)0xC4: token.type = TOKEN_PI; break;
        case (char)0xD1: token.type = TOKEN_PHI; break;
        default:  token.type = TOKEN_NONE; break;
    }

    log_char_token("char", token.type, current);
    
    return token;
}

/* ============================== Node Creation ============================== */

/**
 * Allocates a node from the node pool.
 * Returns NULL if the node pool is full.
 * 
 * @return Pointer to the allocated node, or NULL if the pool is full.
 */
static ExpressionNode* allocate_node(void) {
    if (node_pool_index >= MAX_NODES) {
        return NULL; // Node pool is full
    }
    
    ExpressionNode* node = &node_pool[node_pool_index++];
    memset(node, 0, sizeof(ExpressionNode));
    return node;
}

/**
 * Creates a number node with the given value and position.
 * 
 * @param value The numeric value of the node.
 * @param position The source position of the node.
 * @return Pointer to the created node, or NULL if allocation fails.
 */
static ExpressionNode* create_number_node(double value, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = NODE_NUMBER;
    node->number_value = value;
    node->position = position;
    
    return node;
}

/**
 * Creates a variable node with the given name and position.
 * 
 * @param name The name of the variable.
 * @param position The source position of the node.
 * @return Pointer to the created node, or NULL if allocation fails.
 */
static ExpressionNode* create_variable_node(const char* name, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = NODE_VARIABLE;
    strncpy(node->variable.name, name, MAX_TOKEN_LENGTH - 1);
    node->variable.name[MAX_TOKEN_LENGTH - 1] = '\0';
    node->position = position;
    
    return node;
}

/**
 * Creates a binary operation node.
 * 
 * @param type The type of the binary operation (e.g., addition, subtraction).
 * @param left Pointer to the left operand node.
 * @param right Pointer to the right operand node.
 * @param position The source position of the node.
 * @return Pointer to the created node, or NULL if allocation fails.
 */
static ExpressionNode* create_binary_op_node(NodeType type, ExpressionNode* left, ExpressionNode* right, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = type;
    node->binary_op.left = left;
    node->binary_op.right = right;
    node->position = position;
    
    return node;
}

/**
 * Creates a function node.
 * 
 * @param func_type The type of the function (e.g., sin, cos).
 * @param argument Pointer to the argument node.
 * @param position The source position of the node.
 * @return Pointer to the created node, or NULL if allocation fails.
 */
static ExpressionNode* create_function_node(FunctionType func_type, ExpressionNode* argument, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = NODE_FUNCTION;
    node->function.func_type = func_type;
    node->function.argument = argument;
    node->position = position;
    
    return node;
}

/**
 * Creates a factorial node.
 * 
 * @param expression Pointer to the expression node to apply the factorial to.
 * @param position The source position of the node.
 * @return Pointer to the created node, or NULL if allocation fails.
 */
static ExpressionNode* create_factorial_node(ExpressionNode* expression, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = NODE_FACTORIAL;
    node->factorial.expression = expression;
    node->position = position;
    
    return node;
}

/**
 * Creates a parenthesis node.
 * 
 * @param expression Pointer to the expression node inside the parentheses.
 * @param position The source position of the node.
 * @return Pointer to the created node, or NULL if allocation fails.
 */
static ExpressionNode* create_parenthesis_node(ExpressionNode* expression, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = NODE_PARENTHESIS;
    node->parenthesis.expression = expression;
    node->position = position;
    
    return node;
}

/* ============================== Parser Implementation ============================== */

/**
 * Expects a specific token type and advances, or reports an error.
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @param type The expected token type.
 * @return True if the current token matches the expected type, false otherwise.
 */
static bool expect(Tokenizer* tokenizer, TokenType type) {
    if (tokenizer->current_token.type == type) {
        tokenizer->current_token = get_next_token(tokenizer);
        return true;
    }
    return false;
}

/**
 * Parses an input string into an expression tree.
 * 
 * @param input The input string to parse.
 * @return Pointer to the root of the parsed expression tree, or NULL on error.
 */
ExpressionNode* parse_expression_string(const char* input) {
    // Reset node pool
    node_pool_index = 0;

    log_debug("Parsing expression string");
    log_message("Expression input: %s", input);
    
    // Initialize tokenizer
    Tokenizer tokenizer;
    tokenizer_init(&tokenizer, input);
    
    // Parse the expression
    ExpressionNode* root = parse_expression(&tokenizer);
    if (root == NULL) {
        log_error("Failed to parse expression");
    } else {
        log_debug("Expression parsed successfully");
    }
    return root;
}

/**
 * Parses an expression (addition/subtraction).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed expression node.
 */
static ExpressionNode* parse_expression(Tokenizer* tokenizer) {
    ExpressionNode* left = parse_term(tokenizer);
    
    while (tokenizer->current_token.type == TOKEN_PLUS || 
           tokenizer->current_token.type == TOKEN_MINUS) {
        
        TokenType operator_type = tokenizer->current_token.type;
        SourcePosition operator_position = tokenizer->current_token.position;
        
        // Consume the operator
        tokenizer->current_token = get_next_token(tokenizer);
        
        ExpressionNode* right = parse_term(tokenizer);
        
        if (operator_type == TOKEN_PLUS) {
            left = create_binary_op_node(NODE_ADDITION, left, right, operator_position);
        } else {
            left = create_binary_op_node(NODE_SUBTRACTION, left, right, operator_position);
        }
    }
    
    return left;
}

/**
 * Parses a term (multiplication/division).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed term node.
 */
static ExpressionNode* parse_term(Tokenizer* tokenizer) {
    ExpressionNode* left = parse_factor(tokenizer);
    
    while (tokenizer->current_token.type == TOKEN_MULTIPLY || 
           tokenizer->current_token.type == TOKEN_DIVIDE) {
        
        TokenType operator_type = tokenizer->current_token.type;
        SourcePosition operator_position = tokenizer->current_token.position;
        
        // Consume the operator
        tokenizer->current_token = get_next_token(tokenizer);
        
        ExpressionNode* right = parse_factor(tokenizer);
        
        if (operator_type == TOKEN_MULTIPLY) {
            left = create_binary_op_node(NODE_MULTIPLICATION, left, right, operator_position);
        } else {
            left = create_binary_op_node(NODE_DIVISION, left, right, operator_position);
        }
    }
    
    return left;
}

/**
 * Parses a factor (exponentiation).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed factor node.
 */
static ExpressionNode* parse_factor(Tokenizer* tokenizer) {
    ExpressionNode* left = parse_primary(tokenizer);
    
    if (tokenizer->current_token.type == TOKEN_POWER) {
        SourcePosition operator_position = tokenizer->current_token.position;
        
        // Consume the operator
        tokenizer->current_token = get_next_token(tokenizer);
        
        ExpressionNode* right = parse_factor(tokenizer); // Note: right-associative
        
        left = create_binary_op_node(NODE_EXPONENT, left, right, operator_position);
    }
    
    if (tokenizer->current_token.type == TOKEN_FACTORIAL) {
        SourcePosition operator_position = tokenizer->current_token.position;
        
        // Consume the operator
        tokenizer->current_token = get_next_token(tokenizer);
        
        left = create_factorial_node(left, operator_position);
    }
    
    return left;
}

/**
 * Parses a primary expression (numbers, variables, functions, parenthesized expressions).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed primary expression node.
 */
static ExpressionNode* parse_primary(Tokenizer* tokenizer) {
    Token token = tokenizer->current_token;
    
    switch ((int)token.type) {
        case TOKEN_NUMBER: {
            // Consume the token
            tokenizer->current_token = get_next_token(tokenizer);
            
            // Convert string to double
            double value = atof(token.value);
            
            return create_number_node(value, token.position);
        }

        case TOKEN_PI: {
            // Consume the token
            tokenizer->current_token = get_next_token(tokenizer);
            
            return create_number_node(PI, token.position);
        }

        case TOKEN_PHI: {
            // Consume the token
            tokenizer->current_token = get_next_token(tokenizer);
            
            return create_number_node(PHI, token.position);
        }
        
        case TOKEN_VARIABLE: {
            // Consume the token
            tokenizer->current_token = get_next_token(tokenizer);
            
            return create_variable_node(token.value, token.position);
        }
        
        case TOKEN_FUNCTION: {
            FunctionType func_type;
            
            // Determine which function
            if (strcmp(token.value, "sin") == 0) {
                func_type = FUNC_SIN;
            } else if (strcmp(token.value, "cos") == 0) {
                func_type = FUNC_COS;
            } else if (strcmp(token.value, "tan") == 0) {
                func_type = FUNC_TAN;
            } else if (strcmp(token.value, "log") == 0) {
                func_type = FUNC_LOG;
            } else if (strcmp(token.value, "ln") == 0) {
                func_type = FUNC_LN;
            } else if (strcmp(token.value, "sqrt") == 0) {
                func_type = FUNC_SQRT;
            } else {
                func_type = FUNC_NONE;
            }
            
            // Consume the function name
            tokenizer->current_token = get_next_token(tokenizer);
            
            return parse_function(tokenizer, func_type);
        }
        
        case TOKEN_LEFT_PAREN: {
            // Consume the '('
            tokenizer->current_token = get_next_token(tokenizer);
            
            ExpressionNode* expr = parse_expression(tokenizer);
            
            // Expect a ')'
            if (!expect(tokenizer, TOKEN_RIGHT_PAREN)) {
                // Handle error: missing closing parenthesis
                // In a real implementation, we would report an error
            }
            
            return create_parenthesis_node(expr, token.position);
        }
        
        case TOKEN_MINUS: {
            // Handle unary minus
            SourcePosition operator_position = token.position;
            
            // Consume the '-'
            tokenizer->current_token = get_next_token(tokenizer);
            
            ExpressionNode* expr = parse_factor(tokenizer);
            
            // Create a subtraction with 0 as the left operand
            ExpressionNode* zero = create_number_node(0, operator_position);
            
            return create_binary_op_node(NODE_SUBTRACTION, zero, expr, operator_position);
        }
        
        default:
            // Handle error: unexpected token
            // In a real implementation, we would report an error
            // For now, return a default value
            return create_number_node(0, token.position);
    }
}

/**
 * Parses a function call.
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @param func_type The type of the function being parsed.
 * @return Pointer to the parsed function node.
 */
static ExpressionNode* parse_function(Tokenizer* tokenizer, FunctionType func_type) {
    SourcePosition position = tokenizer->current_token.position;
    
    // Expect a '('
    if (!expect(tokenizer, TOKEN_LEFT_PAREN)) {
        // Handle error: missing opening parenthesis after function name
        // In a real implementation, we would report an error
    }
    
    // Parse the argument expression
    ExpressionNode* argument = parse_expression(tokenizer);
    
    // Expect a ')'
    if (!expect(tokenizer, TOKEN_RIGHT_PAREN)) {
        // Handle error: missing closing parenthesis
        // In a real implementation, we would report an error
    }
    
    return create_function_node(func_type, argument, position);
}

/* ============================== Variable Management ============================== */

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

/* ============================== Function Evaluation ============================== */

/**
 * Evaluates a mathematical function.
 * 
 * @param func_type The type of the function to evaluate.
 * @param argument The argument to the function.
 * @return The result of the function evaluation.
 */
static double evaluate_function(FunctionType func_type, double argument) {
    switch (func_type) {
        case FUNC_SIN:
            return sin(argument);
        
        case FUNC_COS:
            return cos(argument);
        
        case FUNC_TAN:
            return tan(argument);
        
        case FUNC_LOG:
            // Check for domain error
            if (argument <= 0) {
                // Should report an error
                return 0;
            }
            return log10(argument);
        
        case FUNC_LN:
            // Check for domain error
            if (argument <= 0) {
                // Should report an error
                return 0;
            }
            return log(argument);
        
        case FUNC_SQRT:
            // Check for domain error
            if (argument < 0) {
                // Should report an error
                return 0;
            }
            return sqrt(argument);
        
        default:
            return 0;
    }
}

/**
 * Gets the name of a function.
 * 
 * @param func_type The type of the function.
 * @return The name of the function as a string.
 */
static const char* get_function_name(FunctionType func_type) {
    switch (func_type) {
        case FUNC_SIN:  return "sin";
        case FUNC_COS:  return "cos";
        case FUNC_TAN:  return "tan";
        case FUNC_LOG:  return "log";
        case FUNC_LN:   return "ln";
        case FUNC_SQRT: return "sqrt";
        default:        return "unknown";
    }
}

/* ============================== Number Formatting ============================== */

/**
 * Sets the arithmetic mode for calculations.
 * 
 * @param type The arithmetic type (e.g., normal, truncate, round).
 * @param precision The number of decimal places or significant digits.
 * @param use_significant_digits Whether to use significant digits.
 */
void set_arithmetic_mode(ArithmeticType type, int precision, bool use_significant_digits) {
    current_arithmetic_type = type;
    current_precision = precision;
    current_use_significant_digits = use_significant_digits;
}

/**
 * Gets the current arithmetic mode.
 * 
 * @return The current arithmetic type.
 */
ArithmeticType get_arithmetic_mode(void) {
    return current_arithmetic_type;
}

/**
 * Gets the current precision.
 * 
 * @return The current precision setting.
 */
int get_precision(void) {
    return current_precision;
}

/**
 * Gets whether significant digits are used.
 * 
 * @return True if significant digits are used, false otherwise.
 */
bool get_use_significant_digits(void) {
    return current_use_significant_digits;
}

/**
 * Truncates a value to a specific number of decimal places.
 * 
 * @param value The value to truncate.
 * @param decimal_places The number of decimal places to keep.
 * @return The truncated value.
 */
static double truncate_to_decimal_places(double value, int decimal_places) {
    if (decimal_places < 0) {
        decimal_places = 0;
    }
    
    double multiplier = pow(10.0, decimal_places);
    
    if (value >= 0) {
        return floor(value * multiplier) / multiplier;
    } else {
        return ceil(value * multiplier) / multiplier;
    }
}

/**
 * Rounds a value to a specific number of significant digits.
 * 
 * @param value The value to round.
 * @param sig_digits The number of significant digits to keep.
 * @return The rounded value.
 */
static double round_to_significant_digits(double value, int sig_digits) {
    if (sig_digits <= 0) {
        sig_digits = 1;
    }
    
    if (fabs(value) < EPSILON) {
        return 0.0;
    }
    
    // Get the exponent (power of 10) of the value
    int exponent = (int)floor(log10(fabs(value)));
    
    // Calculate the number of decimal places needed
    int decimal_places = sig_digits - exponent - 1;
    
    // Adjust for small numbers (value < 1)
    if (fabs(value) < 1 && value != 0) {
        decimal_places = sig_digits + abs(exponent) - 1;
    }
    
    double multiplier = pow(10.0, decimal_places);
    return round(value * multiplier) / multiplier;
}

/**
 * Applies arithmetic formatting to a value.
 * 
 * @param value The value to format.
 * @return The formatted value.
 */
double apply_arithmetic_format(double value) {
    switch (current_arithmetic_type) {
        case ARITHMETIC_NORMAL:
            return value;
            
        case ARITHMETIC_TRUNCATE:
            if (current_use_significant_digits) {
                // Implement significant digit truncation
                // For simplicity, we'll use rounding for now
                return round_to_significant_digits(value, current_precision);
            } else {
                return truncate_to_decimal_places(value, current_precision);
            }
            
        case ARITHMETIC_ROUND:
            if (current_use_significant_digits) {
                return round_to_significant_digits(value, current_precision);
            } else {
                double multiplier = pow(10.0, current_precision);
                return round(value * multiplier) / multiplier;
            }
            
        default:
            return value;
    }
}

/**
 * Formats a number as a string.
 * 
 * @param value The value to format.
 * @param buffer The buffer to store the formatted string.
 */
void format_number(double value, char* buffer) {
    // Format the number directly without using sprintf
    int integer_part = (int)value;
    
    if (value == (double)integer_part) {
        // It's an integer, format it without decimal
        snprintf(buffer, MAX_TOKEN_LENGTH, "%d", integer_part);
    } else {
        // It has decimal places
        if (current_arithmetic_type == ARITHMETIC_NORMAL) {
            snprintf(buffer, MAX_TOKEN_LENGTH, "%.4f", value);
        } else {
            snprintf(buffer, MAX_TOKEN_LENGTH, "%.*f", current_precision, value);
        }
        
        // Remove trailing zeros
        int len = strlen(buffer);
        if (strchr(buffer, '.') != NULL) {
            while (len > 0 && buffer[len-1] == '0') {
                buffer[--len] = '\0';
            }
            if (len > 0 && buffer[len-1] == '.') {
                buffer[--len] = '\0';
            }
        }
    }
}

/* ============================== Expression Evaluation ============================== */

/**
 * Evaluates an expression node.
 * 
 * @param node Pointer to the expression node to evaluate.
 * @return The result of the evaluation.
 */
double evaluate_expression(ExpressionNode* node) {
    if (node == NULL) {
        log_error("Null expression node");
        return 0;
    }
    
    switch (node->type) {
        case NODE_NUMBER:
            log_message("Evaluating number: %.6f", node->number_value);
            return node->number_value;
        
        case NODE_VARIABLE: {
            bool found;
            double value = get_variable(node->variable.name, &found);
            if (!found) {
                log_error("Undefined variable");
            } else {
                log_message("Variable evaluated: %s = %.6f", node->variable.name, value);
            }
            return value;
        }
        
        case NODE_ADDITION: {
            double left = evaluate_expression(node->binary_op.left);
            double right = evaluate_expression(node->binary_op.right);
            double result = apply_arithmetic_format(left + right);
            log_operation("Addition", result);
            return result;
        }
        
        case NODE_SUBTRACTION: {
            double left = evaluate_expression(node->binary_op.left);
            double right = evaluate_expression(node->binary_op.right);
            double result = apply_arithmetic_format(left - right);
            log_operation("Subtraction", result);
            return result;
        }
        
        case NODE_MULTIPLICATION: {
            double left = evaluate_expression(node->binary_op.left);
            double right = evaluate_expression(node->binary_op.right);
            double result = apply_arithmetic_format(left * right);
            log_operation("Multiplication", result);
            return result;
        }
        
        case NODE_DIVISION: {
            double left = evaluate_expression(node->binary_op.left);
            double right = evaluate_expression(node->binary_op.right);
            if (fabs(right) < EPSILON) {
                log_error("Division by zero");
                return 0;
            }
            double result = apply_arithmetic_format(left / right);
            log_operation("Division", result);
            return result;
        }
        
        case NODE_EXPONENT: {
            double left = evaluate_expression(node->binary_op.left);
            double right = evaluate_expression(node->binary_op.right);
            double result = apply_arithmetic_format(pow(left, right));
            log_operation("Exponentiation", result);
            return result;
        }
        
        case NODE_FUNCTION: {
            double argument = evaluate_expression(node->function.argument);
            double result = apply_arithmetic_format(
                evaluate_function(node->function.func_type, argument)
            );
            log_operation(get_function_name(node->function.func_type), result);
            return result;
        }
        
        case NODE_FACTORIAL: {
            double value = evaluate_expression(node->factorial.expression);
            
            // Check if value is a non-negative integer
            if (value < 0 || fabs(value - round(value)) > EPSILON) {
                // Handle error: factorial is only defined for non-negative integers
                // In a real implementation, we would report an error
                return 0;
            }
            
            int n = (int)round(value);
            double result = 1;
            
            for (int i = 2; i <= n; i++) {
                result *= i;
            }
            
            result = apply_arithmetic_format(result);
            log_operation("Factorial", result);
            return result;
        }
        
        case NODE_PARENTHESIS:
            return evaluate_expression(node->parenthesis.expression);
        
        default:
            log_error("Unknown node type");
            return 0;
    }
}

/**
 * Evaluates an expression string.
 * 
 * @param input The input string to evaluate.
 * @param result Pointer to the structure to store the result.
 * @return True if the evaluation is successful, false otherwise.
 */
bool evaluate_expression_string(const char* input, CalculationResult* result) {
    // Parse the expression
    ExpressionNode* root = parse_expression_string(input);
    if (root == NULL) return false;
    
    // Initialize the result
    memset(result, 0, sizeof(CalculationResult));
    result->arithmetic_mode = current_arithmetic_type;
    result->precision = current_precision;
    result->use_significant_digits = current_use_significant_digits;
    
    // Evaluate with steps
    result->value = evaluate_with_steps(root, result);
    
    // Format the final result
    format_number(result->value, result->formatted_result);
    
    return true;
}

/**
 * Evaluates an expression with step-by-step tracking.
 * 
 * @param node Pointer to the expression node to evaluate.
 * @param result Pointer to the structure to store the evaluation steps.
 * @return The result of the evaluation.
 */
double evaluate_with_steps(ExpressionNode* node, CalculationResult* result) {
    if (node == NULL) return 0;
    
    switch (node->type) {
        case NODE_NUMBER:
            // No step needed for a simple number
            return node->number_value;
        
        case NODE_VARIABLE: {
            bool found;
            double value = get_variable(node->variable.name, &found);
            
            if (found && result->step_count < MAX_STEPS) {
                // Record the variable substitution step
                CalculationStep* step = &result->steps[result->step_count++];
                
                sprintf(step->expression, "%s", node->variable.name);
                sprintf(step->operation, "Substitute variable %s", node->variable.name);
                format_number(value, step->result);
            }
            
            if (!found) {
                // Handle undefined variable
                return 0;
            }
            
            return value;
        }
        
        case NODE_ADDITION: {
            double left = evaluate_with_steps(node->binary_op.left, result);
            double right = evaluate_with_steps(node->binary_op.right, result);
            double operation_result = left + right;
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the addition step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_number(left, left_str);
                format_number(right, right_str);
                
                sprintf(step->expression, "%s + %s", left_str, right_str);
                sprintf(step->operation, "Add %s and %s", left_str, right_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_SUBTRACTION: {
            double left = evaluate_with_steps(node->binary_op.left, result);
            double right = evaluate_with_steps(node->binary_op.right, result);
            double operation_result = left - right;
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the subtraction step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_number(left, left_str);
                format_number(right, right_str);
                
                sprintf(step->expression, "%s - %s", left_str, right_str);
                sprintf(step->operation, "Subtract %s from %s", right_str, left_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_MULTIPLICATION: {
            double left = evaluate_with_steps(node->binary_op.left, result);
            double right = evaluate_with_steps(node->binary_op.right, result);
            double operation_result = left * right;
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the multiplication step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_number(left, left_str);
                format_number(right, right_str);
                
                sprintf(step->expression, "%s * %s", left_str, right_str);
                sprintf(step->operation, "Multiply %s by %s", left_str, right_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_DIVISION: {
            double left = evaluate_with_steps(node->binary_op.left, result);
            double right = evaluate_with_steps(node->binary_op.right, result);
            
            if (fabs(right) < EPSILON) {
                // Division by zero
                if (result->step_count < MAX_STEPS) {
                    CalculationStep* step = &result->steps[result->step_count++];
                    
                    sprintf(step->expression, "Division by zero");
                    sprintf(step->operation, "Error");
                    sprintf(step->result, "Undefined");
                }
                return 0;
            }
            
            double operation_result = left / right;
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the division step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_number(left, left_str);
                format_number(right, right_str);
                
                sprintf(step->expression, "%s / %s", left_str, right_str);
                sprintf(step->operation, "Divide %s by %s", left_str, right_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_EXPONENT: {
            double base = evaluate_with_steps(node->binary_op.left, result);
            double exponent = evaluate_with_steps(node->binary_op.right, result);
            double operation_result = pow(base, exponent);
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the exponentiation step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char base_str[MAX_TOKEN_LENGTH];
                char exponent_str[MAX_TOKEN_LENGTH];
                format_number(base, base_str);
                format_number(exponent, exponent_str);
                
                sprintf(step->expression, "%s ^ %s", base_str, exponent_str);
                sprintf(step->operation, "Raise %s to power %s", base_str, exponent_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_FUNCTION: {
            double argument = evaluate_with_steps(node->function.argument, result);
            const char* func_name = get_function_name(node->function.func_type);
            
            // Handle domain errors
            bool domain_error = false;
            if ((node->function.func_type == FUNC_LOG || node->function.func_type == FUNC_LN) && argument <= 0) {
                domain_error = true;
            } else if (node->function.func_type == FUNC_SQRT && argument < 0) {
                domain_error = true;
            }
            
            if (domain_error) {
                if (result->step_count < MAX_STEPS) {
                    CalculationStep* step = &result->steps[result->step_count++];
                    
                    sprintf(step->expression, "%s domain error", func_name);
                    sprintf(step->operation, "Error");
                    sprintf(step->result, "Undefined");
                }
                return 0;
            }
            
            double operation_result = evaluate_function(node->function.func_type, argument);
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the function evaluation step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char arg_str[MAX_TOKEN_LENGTH];
                format_number(argument, arg_str);
                
                sprintf(step->expression, "%s(%s)", func_name, arg_str);
                sprintf(step->operation, "Calculate %s of %s", func_name, arg_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_FACTORIAL: {
            double expression_value = evaluate_with_steps(node->factorial.expression, result);
            
            // Check if value is a non-negative integer
            if (expression_value < 0 || fabs(expression_value - round(expression_value)) > EPSILON) {
                if (result->step_count < MAX_STEPS) {
                    CalculationStep* step = &result->steps[result->step_count++];
                    
                    sprintf(step->expression, "Factorial domain error");
                    sprintf(step->operation, "Error");
                    sprintf(step->result, "Undefined");
                }
                return 0;
            }
            
            int n = (int)round(expression_value);
            double operation_result = 1;
            
            for (int i = 2; i <= n; i++) {
                operation_result *= i;
            }
            
            double formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the factorial step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char expr_str[MAX_TOKEN_LENGTH];
                format_number(expression_value, expr_str);
                
                sprintf(step->expression, "%s!", expr_str);
                sprintf(step->operation, "Calculate factorial of %s", expr_str);
                format_number(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_PARENTHESIS: {
            // Evaluate the expression inside the parentheses
            double value = evaluate_with_steps(node->parenthesis.expression, result);
            
            // We don't add a separate step for parentheses
            return value;
        }
        
        default:
            // Should never happen
            return 0;
    }
}
