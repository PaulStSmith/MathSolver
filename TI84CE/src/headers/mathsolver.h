/**
 * MathSolver for TI-84 CE - Core Definitions
 * 
 * Contains all type definitions, constants, and function prototypes
 */

#ifndef MATHSOLVER_H
#define MATHSOLVER_H

#include <stdbool.h>

/* ============================== Constants ============================== */

#define MAX_INPUT_LENGTH     100   // Maximum input expression length
#define MAX_NODES            50    // Maximum nodes in the expression tree
#define MAX_TOKEN_LENGTH     20    // Maximum token length
#define MAX_VARIABLES        10    // Maximum number of variables
#define MAX_STEPS            20    // Maximum calculation steps to display
#define SCREEN_WIDTH         320   // Calculator screen width
#define SCREEN_HEIGHT        240   // Calculator screen height
#define EPSILON              1e-10 // Small value for comparisons

/* Mathematical constants */
#define PI  3.14159265358979323846
#define E   2.71828182845904523536
#define PHI 1.61803398874989484820

/* Colors */
#define COLOR_BACKGROUND     0xFF   // White
#define COLOR_TEXT           0x00   // Black
#define COLOR_HIGHLIGHT      0x03   // Green
#define COLOR_ERROR          0xE0   // Red
#define COLOR_GRIDLINE       0xB5   // Light gray

/* Token types */
typedef enum {
    TOKEN_NONE,
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_POWER,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_COMMA,
    TOKEN_FACTORIAL,
    TOKEN_FUNCTION,
    TOKEN_END
} TokenType;

/* Node types */
typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_ADDITION,
    NODE_SUBTRACTION,
    NODE_MULTIPLICATION,
    NODE_DIVISION,
    NODE_EXPONENT,
    NODE_FUNCTION,
    NODE_FACTORIAL,
    NODE_PARENTHESIS
} NodeType;

/* Function types */
typedef enum {
    FUNC_SIN,
    FUNC_COS,
    FUNC_TAN,
    FUNC_LOG,
    FUNC_LN,
    FUNC_SQRT,
    FUNC_NONE
} FunctionType;

/* Arithmetic types for formatting */
typedef enum {
    ARITHMETIC_NORMAL,
    ARITHMETIC_TRUNCATE,
    ARITHMETIC_ROUND
} ArithmeticType;

/* ============================== Structures ============================== */

/**
 * Source position structure for error reporting
 */
typedef struct {
    int start;  // Start index in the input string
    int end;    // End index in the input string
    int line;   // Line number
    int column; // Column number
} SourcePosition;

/**
 * Token structure representing a parsed token
 */
typedef struct {
    TokenType type;              // Type of token
    char value[MAX_TOKEN_LENGTH]; // Token value as string
    SourcePosition position;     // Position in the input
} Token;

/**
 * Forward declaration of ExpressionNode for use in function signatures
 */
typedef struct ExpressionNode ExpressionNode;

/**
 * Expression node structure (the building block of the AST)
 */
struct ExpressionNode {
    NodeType type;               // Type of node
    SourcePosition position;     // Position in source
    
    // Node specific data depends on type
    union {
        // Number node
        double number_value;
        
        // Variable node
        struct {
            char name[MAX_TOKEN_LENGTH];
        } variable;
        
        // Binary operation node (add, subtract, multiply, divide, power)
        struct {
            ExpressionNode* left;
            ExpressionNode* right;
        } binary_op;
        
        // Function node
        struct {
            FunctionType func_type;
            ExpressionNode* argument;
        } function;
        
        // Factorial node
        struct {
            ExpressionNode* expression;
        } factorial;
        
        // Parenthesis node
        struct {
            ExpressionNode* expression;
        } parenthesis;
    };
};

/**
 * Tokenizer structure for parsing input
 */
typedef struct {
    const char* input;   // Input string
    int position;        // Current position in the input
    int line;            // Current line number
    int column;          // Current column number
    Token current_token; // Current token
} Tokenizer;

/**
 * Variable structure to store variable name and value
 */
typedef struct {
    char name[MAX_TOKEN_LENGTH];
    double value;
    bool is_defined;
} Variable;

/**
 * Calculation step structure for step-by-step output
 */
typedef struct {
    char expression[MAX_INPUT_LENGTH];
    char operation[MAX_INPUT_LENGTH];
    char result[MAX_INPUT_LENGTH];
} CalculationStep;

/**
 * Calculation result structure
 */
typedef struct {
    double value;                    // Final value
    int step_count;                  // Number of steps
    CalculationStep steps[MAX_STEPS]; // Step-by-step calculation
    char formatted_result[MAX_TOKEN_LENGTH]; // Formatted result string
    ArithmeticType arithmetic_mode;  // Arithmetic mode used
    int precision;                   // Precision used
    bool use_significant_digits;     // Whether significant digits were used
} CalculationResult;

/* ============================== Function Prototypes ============================== */

// Initialization and cleanup
void mathsolver_init(void);
void mathsolver_cleanup(void);

// Tokenization and parsing
void tokenizer_init(Tokenizer* tokenizer, const char* input);
Token get_next_token(Tokenizer* tokenizer);
ExpressionNode* parse_expression_string(const char* input);
static ExpressionNode* parse_expression(Tokenizer* tokenizer);

// Evaluation
bool evaluate_expression_string(const char* input, CalculationResult* result);
double evaluate_expression(ExpressionNode* node);
double evaluate_with_steps(ExpressionNode* node, CalculationResult* result);

// Variable management
void set_variable(const char* name, double value);
double get_variable(const char* name, bool* found);
bool is_constant(const char* name);

// Formatting
void format_number(double value, char* buffer);
void format_expression(ExpressionNode* node, char* buffer);
double apply_arithmetic_format(double value);

// Settings
void set_arithmetic_mode(ArithmeticType type, int precision, bool use_significant_digits);
ArithmeticType get_arithmetic_mode(void);
int get_precision(void);
bool get_use_significant_digits(void);

#endif /* MATHSOLVER_H */
