/**
 * MathSolver for TI-84 CE - Core Definitions
 * 
 * Contains all type definitions, constants, and function prototypes
 */

#ifndef MATHSOLVER_H
#define MATHSOLVER_H

#include <stdbool.h>

/* ============================== Constants ============================== */

/** Maximum input expression length */
#define MAX_INPUT_LENGTH     100

/** Maximum nodes in the expression tree */
#define MAX_NODES            50

/** Maximum token length */
#define MAX_TOKEN_LENGTH     20

/** Maximum number of variables */
#define MAX_VARIABLES        10

/** Maximum calculation steps to display */
#define MAX_STEPS            20

/** Calculator screen width */
#define SCREEN_WIDTH         320

/** Calculator screen height */
#define SCREEN_HEIGHT        240

/** Small value for comparisons */
#define EPSILON              1e-10

/* Mathematical constants */
/** Value of Pi */
#define PI  3.14159265358979323846

/** Base of natural logarithm (e) */
#define E   2.71828182845904523536

/** Golden ratio (phi) */
#define PHI 1.61803398874989484820

/* Colors */
/** Background color (white) */
#define COLOR_BACKGROUND     0xFF

/** Text color (black) */
#define COLOR_TEXT           0x00

/** Highlight color (green) */
#define COLOR_HIGHLIGHT      0x03

/** Error color (red) */
#define COLOR_ERROR          0xE0

/** Gridline color (light gray) */
#define COLOR_GRIDLINE       0xB5

/* ============================== Types ============================== */

/**
 * Enumeration of token types for parsing
 */
typedef enum {
    TOKEN_NONE,         /**< No token */
    TOKEN_NUMBER,       /**< Numeric token */
    TOKEN_VARIABLE,     /**< Variable token */
    TOKEN_PLUS,         /**< Plus operator */
    TOKEN_MINUS,        /**< Minus operator */
    TOKEN_MULTIPLY,     /**< Multiplication operator */
    TOKEN_DIVIDE,       /**< Division operator */
    TOKEN_POWER,        /**< Power operator */
    TOKEN_LEFT_PAREN,   /**< Left parenthesis */
    TOKEN_RIGHT_PAREN,  /**< Right parenthesis */
    TOKEN_COMMA,        /**< Comma separator */
    TOKEN_FACTORIAL,    /**< Factorial operator */
    TOKEN_FUNCTION,     /**< Function token */
    TOKEN_END           /**< End of input */
} TokenType;

/**
 * Enumeration of node types for the abstract syntax tree (AST)
 */
typedef enum {
    NODE_NUMBER,        /**< Number node */
    NODE_VARIABLE,      /**< Variable node */
    NODE_ADDITION,      /**< Addition node */
    NODE_SUBTRACTION,   /**< Subtraction node */
    NODE_MULTIPLICATION,/**< Multiplication node */
    NODE_DIVISION,      /**< Division node */
    NODE_EXPONENT,      /**< Exponentiation node */
    NODE_FUNCTION,      /**< Function node */
    NODE_FACTORIAL,     /**< Factorial node */
    NODE_PARENTHESIS    /**< Parenthesis node */
} NodeType;

/**
 * Enumeration of supported mathematical functions
 */
typedef enum {
    FUNC_SIN,           /**< Sine function */
    FUNC_COS,           /**< Cosine function */
    FUNC_TAN,           /**< Tangent function */
    FUNC_LOG,           /**< Logarithm (base 10) */
    FUNC_LN,            /**< Natural logarithm */
    FUNC_SQRT,          /**< Square root */
    FUNC_NONE           /**< No function */
} FunctionType;

/**
 * Enumeration of arithmetic formatting modes
 */
typedef enum {
    ARITHMETIC_NORMAL,  /**< Normal formatting */
    ARITHMETIC_TRUNCATE,/**< Truncate decimals */
    ARITHMETIC_ROUND    /**< Round decimals */
} ArithmeticType;

/* ============================== Structures ============================== */

/**
 * Source position structure for error reporting
 */
typedef struct {
    int start;  /**< Start index in the input string */
    int end;    /**< End index in the input string */
    int line;   /**< Line number */
    int column; /**< Column number */
} SourcePosition;

/**
 * Token structure representing a parsed token
 */
typedef struct {
    TokenType type;              /**< Type of token */
    char value[MAX_TOKEN_LENGTH];/**< Token value as string */
    SourcePosition position;     /**< Position in the input */
} Token;

/**
 * Forward declaration of ExpressionNode for use in function signatures
 */
typedef struct ExpressionNode ExpressionNode;

/**
 * Expression node structure (the building block of the AST)
 */
struct ExpressionNode {
    NodeType type;               /**< Type of node */
    SourcePosition position;     /**< Position in source */
    
    union {
        double number_value;     /**< Value for number nodes */
        struct {
            char name[MAX_TOKEN_LENGTH]; /**< Variable name */
        } variable;
        struct {
            ExpressionNode* left; /**< Left operand */
            ExpressionNode* right;/**< Right operand */
        } binary_op;
        struct {
            FunctionType func_type; /**< Function type */
            ExpressionNode* argument; /**< Function argument */
        } function;
        struct {
            ExpressionNode* expression; /**< Expression for factorial */
        } factorial;
        struct {
            ExpressionNode* expression; /**< Expression in parentheses */
        } parenthesis;
    };
};

/**
 * Tokenizer structure for parsing input
 */
typedef struct {
    const char* input;   /**< Input string */
    int position;        /**< Current position in the input */
    int line;            /**< Current line number */
    int column;          /**< Current column number */
    Token current_token; /**< Current token */
} Tokenizer;

/**
 * Variable structure to store variable name and value
 */
typedef struct {
    char name[MAX_TOKEN_LENGTH]; /**< Variable name */
    double value;                /**< Variable value */
    bool is_defined;             /**< Whether the variable is defined */
} Variable;

/**
 * Calculation step structure for step-by-step output
 */
typedef struct {
    char expression[MAX_INPUT_LENGTH]; /**< Expression at this step */
    char operation[MAX_INPUT_LENGTH];  /**< Operation performed */
    char result[MAX_INPUT_LENGTH];     /**< Result of the operation */
} CalculationStep;

/**
 * Calculation result structure
 */
typedef struct {
    double value;                    /**< Final value */
    int step_count;                  /**< Number of steps */
    CalculationStep steps[MAX_STEPS];/**< Step-by-step calculation */
    char formatted_result[MAX_TOKEN_LENGTH]; /**< Formatted result string */
    ArithmeticType arithmetic_mode;  /**< Arithmetic mode used */
    int precision;                   /**< Precision used */
    bool use_significant_digits;     /**< Whether significant digits were used */
} CalculationResult;

/* ============================== Function Prototypes ============================== */

/**
 * Initialize the MathSolver library
 */
void mathsolver_init(void);

/**
 * Clean up resources used by the MathSolver library
 */
void mathsolver_cleanup(void);

/**
 * Initialize the tokenizer with the given input string
 * @param tokenizer Pointer to the tokenizer structure
 * @param input Input string to tokenize
 */
void tokenizer_init(Tokenizer* tokenizer, const char* input);

/**
 * Get the next token from the tokenizer
 * @param tokenizer Pointer to the tokenizer structure
 * @return The next token
 */
Token get_next_token(Tokenizer* tokenizer);

/**
 * Parse an input string into an expression tree
 * @param input Input string to parse
 * @return Pointer to the root of the expression tree
 */
ExpressionNode* parse_expression_string(const char* input);

/**
 * Parse an expression from the tokenizer
 * @param tokenizer Pointer to the tokenizer structure
 * @return Pointer to the root of the parsed expression tree
 */
static ExpressionNode* parse_expression(Tokenizer* tokenizer);

/**
 * Parses a term (multiplication/division).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed term node.
 */
static ExpressionNode* parse_term(Tokenizer* tokenizer);

/**
 * Parses a factor (exponentiation).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed factor node.
 */
static ExpressionNode* parse_factor(Tokenizer* tokenizer);

/**
 * Parses a primary expression (numbers, variables, functions, parenthesized expressions).
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @return Pointer to the parsed primary expression node.
 */
static ExpressionNode* parse_primary(Tokenizer* tokenizer);

/**
 * Parses a function call.
 * 
 * @param tokenizer Pointer to the tokenizer.
 * @param func_type The type of the function being parsed.
 * @return Pointer to the parsed function node.
 */
static ExpressionNode* parse_function(Tokenizer* tokenizer, FunctionType func_type);

/**
 * Evaluate an expression string and store the result
 * @param input Input string to evaluate
 * @param result Pointer to the result structure
 * @return True if evaluation was successful, false otherwise
 */
bool evaluate_expression_string(const char* input, CalculationResult* result);

/**
 * Evaluate an expression tree
 * @param node Pointer to the root of the expression tree
 * @return The evaluated result
 */
double evaluate_expression(ExpressionNode* node);

/**
 * Evaluate an expression tree with step-by-step output
 * @param node Pointer to the root of the expression tree
 * @param result Pointer to the result structure
 * @return The evaluated result
 */
double evaluate_with_steps(ExpressionNode* node, CalculationResult* result);

/**
 * Set the value of a variable
 * @param name Name of the variable
 * @param value Value to assign to the variable
 */
void set_variable(const char* name, double value);

/**
 * Get the value of a variable
 * @param name Name of the variable
 * @param found Pointer to a boolean indicating if the variable was found
 * @return The value of the variable
 */
double get_variable(const char* name, bool* found);

/**
 * Check if a name corresponds to a mathematical constant
 * @param name Name to check
 * @return True if the name is a constant, false otherwise
 */
bool is_constant(const char* name);

/**
 * Format a number into a string
 * @param value Number to format
 * @param buffer Buffer to store the formatted string
 */
void format_number(double value, char* buffer);

/**
 * Format an expression tree into a string
 * @param node Pointer to the root of the expression tree
 * @param buffer Buffer to store the formatted string
 */
void format_expression(ExpressionNode* node, char* buffer);

/**
 * Apply arithmetic formatting to a number
 * @param value Number to format
 * @return The formatted number
 */
double apply_arithmetic_format(double value);

/**
 * Set the arithmetic mode and precision
 * @param type Arithmetic mode to set
 * @param precision Number of decimal places or significant digits
 * @param use_significant digits Whether to use significant digits
 */
void set_arithmetic_mode(ArithmeticType type, int precision, bool use_significant_digits);

/**
 * Get the current arithmetic mode
 * @return The current arithmetic mode
 */
ArithmeticType get_arithmetic_mode(void);

/**
 * Get the current precision setting
 * @return The current precision
 */
int get_precision(void);

/**
 * Check if significant digits are being used
 * @return True if significant digits are used, false otherwise
 */
bool get_use_significant_digits(void);

#endif /* MATHSOLVER_H */
