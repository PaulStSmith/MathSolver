#include <stdlib.h>
#include "headers/log.h"
#include "headers/mathsolver.h"
#include "headers/parser_private.h"

/**
 * Static memory pool for expression nodes.
 * Used to allocate nodes for the expression tree.
 */
static ExpressionNode node_pool[MAX_NODES];

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
            
            // Convert string to real_t
            real_t value = os_StrToReal(token.value, NULL);
            
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
            ExpressionNode* zero = create_number_node(ZERO, operator_position);
            
            return create_binary_op_node(NODE_SUBTRACTION, zero, expr, operator_position);
        }
        
        default:
            // Handle error: unexpected token
            // In a real implementation, we would report an error
            // For now, return a default value
            return create_number_node(ZERO, token.position);
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
static ExpressionNode* create_number_node(real_t value, SourcePosition position) {
    ExpressionNode* node = allocate_node();
    if (node == NULL) return NULL;
    
    node->type = NODE_NUMBER;
    node->number_value = value;
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

