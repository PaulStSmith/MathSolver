
/*
 * This is an automatically generated header file from the source file: .\src\mathsolver.c
 * Do not modify this file directly. Instead, modify the source file and regenerate this header.
 * 
 * Generated on: 03/21/2025 11:55:27
 * Source file size: 43.92 KB
 * Source file lines: 1298
 * Function prototypes found: 37
 */

#ifndef MATHSOLVER_FUNCTIONS_H
#define MATHSOLVER_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

// Function Prototypes
void mathsolver_init(void);
void mathsolver_cleanup(void);
static void advance_position(Tokenizer* tokenizer);
static void skip_whitespace(Tokenizer* tokenizer);
void tokenizer_init(Tokenizer* tokenizer, const char* input);
Token get_next_token(Tokenizer* tokenizer);
static ExpressionNode* allocate_node(void);
static ExpressionNode* create_number_node(double value, SourcePosition position);
static ExpressionNode* create_variable_node(const char* name, SourcePosition position);
static ExpressionNode* create_binary_op_node(NodeType type, ExpressionNode* left, ExpressionNode* right, SourcePosition position);
static ExpressionNode* create_function_node(FunctionType func_type, ExpressionNode* argument, SourcePosition position);
static ExpressionNode* create_factorial_node(ExpressionNode* expression, SourcePosition position);
static ExpressionNode* create_parenthesis_node(ExpressionNode* expression, SourcePosition position);
static bool match_and_consume(Tokenizer* tokenizer, TokenType type);
static bool expect(Tokenizer* tokenizer, TokenType type);
ExpressionNode* parse_expression_string(const char* input);
static ExpressionNode* parse_expression(Tokenizer* tokenizer);
static ExpressionNode* parse_term(Tokenizer* tokenizer);
static ExpressionNode* parse_factor(Tokenizer* tokenizer);
static ExpressionNode* parse_primary(Tokenizer* tokenizer);
static ExpressionNode* parse_function(Tokenizer* tokenizer, FunctionType func_type);
void set_variable(const char* name, double value);
double get_variable(const char* name, bool* found);
bool is_constant(const char* name);
static double evaluate_function(FunctionType func_type, double argument);
static const char* get_function_name(FunctionType func_type);
void set_arithmetic_mode(ArithmeticType type, int precision, bool use_significant_digits);
ArithmeticType get_arithmetic_mode(void);
int get_precision(void);
bool get_use_significant_digits(void);
static double truncate_to_decimal_places(double value, int decimal_places);
static double round_to_significant_digits(double value, int sig_digits);
double apply_arithmetic_format(double value);
void format_number(double value, char* buffer);
double evaluate_expression(ExpressionNode* node);
bool evaluate_expression_string(const char* input, CalculationResult* result);
double evaluate_with_steps(ExpressionNode* node, CalculationResult* result);

#ifdef __cplusplus
}
#endif

#endif // MATHSOLVER_FUNCTIONS_H
