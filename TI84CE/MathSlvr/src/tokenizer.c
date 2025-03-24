#include "headers/log.h"
#include "headers/mathsolver.h"
#include "headers/tokenizer_public.h"

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

    log_token_char("char", token.type, current);
    
    return token;
}
