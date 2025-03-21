#include <stdio.h>
#include <math.h>
#include "headers/log.h"
#include "headers/mathsolver.h"
#include "headers/evaluator_private.h"

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
