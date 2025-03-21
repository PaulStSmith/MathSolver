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
    TOKEN_END,          /**< End of input */
    TOKEN_PI,           /**< Pi constant */
    TOKEN_PHI           /**< Phi constant */
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

extern ArithmeticType current_arithmetic_type;
extern int current_precision;
extern bool current_use_significant_digits;

/**
 * Array of variables used in expressions.
 * Stores user-defined variables and their values.
 */
extern Variable variables[MAX_VARIABLES];

/** Number of currently defined variables. */
extern int variable_count;

/** Index of the next available node in the node pool. */
extern int node_pool_index;

#include "mathsolver_public.h"
#include "arithmetic_public.h"
#include "evaluator_public.h"
#include "parser_public.h"
#include "tokenizer_public.h"
#include "variables_public.h"

#endif /* MATHSOLVER_H */
