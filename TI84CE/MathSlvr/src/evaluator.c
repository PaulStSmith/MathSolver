#include <stdio.h>
#include <string.h>
#include <tice.h>
#include <ti/real.h>
#include "headers/log.h"
#include "headers/mathsolver.h"
#include "headers/evaluator_private.h"

/* ============================== Expression Evaluation ============================== */

/*
 *  ___                        _            ___          _           _   _          
 * | __|_ ___ __ _ _ ___ _____(_)___ _ _   | __|_ ____ _| |_  _ __ _| |_(_)___ _ _  
 * | _|\ \ / '_ \ '_/ -_|_-<_-< / _ \ ' \  | _|\ V / _` | | || / _` |  _| / _ \ ' \ 
 * |___/_\_\ .__/_| \___/__/__/_\___/_||_| |___|\_/\__,_|_|\_,_\__,_|\__|_\___/_||_|
 *         |_|                                                                      
 * 
 * The following functions are used to evaluate mathematical expressions.
 */

/**
 * Evaluates an expression node.
 * 
 * @param node Pointer to the expression node to evaluate.
 * @return The result of the evaluation.
 */
real_t evaluate_expression(ExpressionNode* node) {
    if (node == NULL) {
        log_error("Null expression node");
        real_t zero = os_Int24ToReal(0);
        return zero;
    }
    
    switch (node->type) {
        case NODE_NUMBER: {
            real_t value = node->number_value;
            real_t result = apply_arithmetic_format(value);
            
            char value_str[MAX_TOKEN_LENGTH];
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(value, value_str);
            format_real(result, rst_str);
            log_message("Evaluating number: %s -> %s", value_str, rst_str);
            
            return result;
        }
        
        case NODE_VARIABLE: {
            bool found;
            real_t value = get_variable(node->variable.name, &found);
            real_t result = apply_arithmetic_format(value);
            
            if (!found) {
                log_error("Undefined variable");
            } else {
                char value_str[MAX_TOKEN_LENGTH];
                char rst_str[MAX_TOKEN_LENGTH];
                format_real(value, value_str);
                format_real(result, rst_str);
                log_message("Variable evaluated: %s = %s -> %s", node->variable.name, value_str, rst_str);
            }
            
            return result;
        }
        
        case NODE_ADDITION: {
            real_t left = evaluate_expression(node->binary_op.left);
            real_t right = evaluate_expression(node->binary_op.right);
            real_t result = apply_arithmetic_format(os_RealAdd(&left, &right));
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation("Addition", rst_str);
            
            return result;
        }
        
        case NODE_SUBTRACTION: {
            real_t left = evaluate_expression(node->binary_op.left);
            real_t right = evaluate_expression(node->binary_op.right);
            real_t result = apply_arithmetic_format(os_RealSub(&left, &right));
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation("Subtraction", rst_str);
            
            return result;
        }
        
        case NODE_MULTIPLICATION: {
            real_t left = evaluate_expression(node->binary_op.left);
            real_t right = evaluate_expression(node->binary_op.right);
            real_t result = apply_arithmetic_format(os_RealMul(&left, &right));
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation("Multiplication", rst_str);
            
            return result;
        }
        
        case NODE_DIVISION: {
            real_t left = evaluate_expression(node->binary_op.left);
            real_t right = evaluate_expression(node->binary_op.right);
            
            real_t zero = os_Int24ToReal(0);
            if (os_RealCompare(&right, &zero) == 0) {
                log_error("Division by zero");
                return zero;
            }
            
            real_t result = apply_arithmetic_format(os_RealDiv(&left, &right));
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation("Division", rst_str);
            
            return result;
        }
        
        case NODE_EXPONENT: {
            real_t left = evaluate_expression(node->binary_op.left);
            real_t right = evaluate_expression(node->binary_op.right);
            real_t result = apply_arithmetic_format(os_RealPow(&left, &right));
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation("Exponentiation", rst_str);
            
            return result;
        }
        
        case NODE_FUNCTION: {
            real_t argument = evaluate_expression(node->function.argument);
            real_t result = apply_arithmetic_format(
                evaluate_function(node->function.func_type, argument)
            );
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation(get_function_name(node->function.func_type), rst_str);
            
            return result;
        }
        
        case NODE_FACTORIAL: {
            real_t value = evaluate_expression(node->factorial.expression);
            
            // Check if value is a non-negative integer
            real_t rounded = os_RealRoundInt(&value);
            real_t zero = os_Int24ToReal(0);
            
            if (os_RealCompare(&value, &zero) < 0 || 
                os_RealCompare(&value, &rounded) != 0) {
                // Handle error: factorial is only defined for non-negative integers
                log_error("Factorial is only defined for non-negative integers");
                return zero;
            }
            
            int24_t n = os_RealToInt24(&value);
            real_t result = os_Int24ToReal(1);
            
            for (int i = 2; i <= n; i++) {
                real_t i_real = os_Int24ToReal(i);
                result = os_RealMul(&result, &i_real);
            }
            
            result = apply_arithmetic_format(result);
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(result, rst_str);
            log_operation("Factorial", rst_str);
            
            return result;
        }
        
        case NODE_PARENTHESIS:
            return evaluate_expression(node->parenthesis.expression);
        
        default:
            log_error("Unknown node type");
            real_t zero = os_Int24ToReal(0);
            return zero;
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
    log_message("Beginning expression evaluation.");
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
    format_real(result->value, result->formatted_result);
    
    return true;
}

char* node_to_string(ExpressionNode* node) {
    static char buffer[MAX_INPUT_LENGTH];
    char temp[MAX_TOKEN_LENGTH];
    
    switch (node->type) {
        case NODE_NUMBER:
            format_real(node->number_value, temp);
            sprintf(buffer, "%s", temp);
            break;
        case NODE_VARIABLE:
            sprintf(buffer, "%s", node->variable.name);
            break;
        case NODE_ADDITION:
            sprintf(buffer, "(%s + %s)", node_to_string(node->binary_op.left), node_to_string(node->binary_op.right));
            break;
        case NODE_SUBTRACTION:
            sprintf(buffer, "(%s - %s)", node_to_string(node->binary_op.left), node_to_string(node->binary_op.right));
            break;
        case NODE_MULTIPLICATION:
            sprintf(buffer, "(%s * %s)", node_to_string(node->binary_op.left), node_to_string(node->binary_op.right));
            break;
        case NODE_DIVISION:
            sprintf(buffer, "(%s / %s)", node_to_string(node->binary_op.left), node_to_string(node->binary_op.right));
            break;
        case NODE_EXPONENT:
            sprintf(buffer, "(%s ^ %s)", node_to_string(node->binary_op.left), node_to_string(node->binary_op.right));
            break;
        case NODE_FUNCTION:
            sprintf(buffer, "%s(%s)", get_function_name(node->function.func_type), node_to_string(node->function.argument));
            break;
        case NODE_FACTORIAL:
            sprintf(buffer, "%s!", node_to_string(node->factorial.expression));
            break;
        case NODE_PARENTHESIS:
            sprintf(buffer, "(%s)", node_to_string(node->parenthesis.expression));
            break;
        default:
            sprintf(buffer, "Unknown node type");
            break;
    }
    return buffer;
}

char* result_to_string(CalculationResult* result) {
    static char buffer[MAX_INPUT_LENGTH];
    char value_str[MAX_TOKEN_LENGTH];
    format_real(result->value, value_str);
    sprintf(buffer, "Value: %s, Step: %d, Result: %s", value_str, result->step_count, result->formatted_result);
    return buffer;
}

/**
 * Evaluates an expression with step-by-step tracking.
 * 
 * @param node Pointer to the expression node to evaluate.
 * @param result Pointer to the structure to store the evaluation steps.
 * @return The result of the evaluation.
 */
real_t evaluate_with_steps(ExpressionNode* node, CalculationResult* result) {
    log_debug("Evaluating expression with steps.");
    log_message("Node: %s", node_to_string(node));
    log_message("Result: %s", result_to_string(result));    

    if (node == NULL) {
        real_t zero = os_Int24ToReal(0);
        return zero;
    }
    
    switch (node->type) {
        case NODE_NUMBER: {
            real_t value = node->number_value;
            real_t rst = apply_arithmetic_format(value);
            
            char rst_str[MAX_TOKEN_LENGTH];
            format_real(rst, rst_str);
            log_operation("Number", rst_str);
            
            // No step needed for a simple number
            return rst;
        }
        
        case NODE_VARIABLE: {
            bool found;
            real_t value = get_variable(node->variable.name, &found);
            
            if (found && result->step_count < MAX_STEPS) {
                // Record the variable substitution step
                CalculationStep* step = &result->steps[result->step_count++];
                
                sprintf(step->expression, "%s", node->variable.name);
                sprintf(step->operation, "Substitute variable %s", node->variable.name);
                format_real(value, step->result);
            }
            
            if (!found) {
                // Handle undefined variable
                real_t zero = os_Int24ToReal(0);
                return zero;
            }
            
            return value;
        }
        
        case NODE_ADDITION: {
            real_t left = evaluate_with_steps(node->binary_op.left, result);
            real_t right = evaluate_with_steps(node->binary_op.right, result);
            real_t operation_result = os_RealAdd(&left, &right);
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the addition step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_real(left, left_str);
                format_real(right, right_str);
                
                sprintf(step->expression, "%s + %s", left_str, right_str);
                sprintf(step->operation, "Add %s and %s", left_str, right_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_SUBTRACTION: {
            real_t left = evaluate_with_steps(node->binary_op.left, result);
            real_t right = evaluate_with_steps(node->binary_op.right, result);
            real_t operation_result = os_RealSub(&left, &right);
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the subtraction step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_real(left, left_str);
                format_real(right, right_str);
                
                sprintf(step->expression, "%s - %s", left_str, right_str);
                sprintf(step->operation, "Subtract %s from %s", right_str, left_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_MULTIPLICATION: {
            real_t left = evaluate_with_steps(node->binary_op.left, result);
            real_t right = evaluate_with_steps(node->binary_op.right, result);
            real_t operation_result = os_RealMul(&left, &right);
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the multiplication step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_real(left, left_str);
                format_real(right, right_str);
                
                sprintf(step->expression, "%s * %s", left_str, right_str);
                sprintf(step->operation, "Multiply %s by %s", left_str, right_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_DIVISION: {
            real_t left = evaluate_with_steps(node->binary_op.left, result);
            real_t right = evaluate_with_steps(node->binary_op.right, result);
            
            real_t zero = os_Int24ToReal(0);
            if (os_RealCompare(&right, &zero) == 0) {
                // Division by zero
                if (result->step_count < MAX_STEPS) {
                    CalculationStep* step = &result->steps[result->step_count++];
                    
                    sprintf(step->expression, "Division by zero");
                    sprintf(step->operation, "Error");
                    sprintf(step->result, "Undefined");
                }
                return zero;
            }
            
            real_t operation_result = os_RealDiv(&left, &right);
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the division step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char left_str[MAX_TOKEN_LENGTH];
                char right_str[MAX_TOKEN_LENGTH];
                format_real(left, left_str);
                format_real(right, right_str);
                
                sprintf(step->expression, "%s / %s", left_str, right_str);
                sprintf(step->operation, "Divide %s by %s", left_str, right_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_EXPONENT: {
            real_t base = evaluate_with_steps(node->binary_op.left, result);
            real_t exponent = evaluate_with_steps(node->binary_op.right, result);
            real_t operation_result = os_RealPow(&base, &exponent);
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the exponentiation step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char base_str[MAX_TOKEN_LENGTH];
                char exponent_str[MAX_TOKEN_LENGTH];
                format_real(base, base_str);
                format_real(exponent, exponent_str);
                
                sprintf(step->expression, "%s ^ %s", base_str, exponent_str);
                sprintf(step->operation, "Raise %s to power %s", base_str, exponent_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_FUNCTION: {
            real_t argument = evaluate_with_steps(node->function.argument, result);
            const char* func_name = get_function_name(node->function.func_type);
            
            // Handle domain errors
            bool domain_error = false;
            real_t zero = os_Int24ToReal(0);
            
            if ((node->function.func_type == FUNC_LOG || node->function.func_type == FUNC_LN) && 
                os_RealCompare(&argument, &zero) <= 0) {
                domain_error = true;
            } else if (node->function.func_type == FUNC_SQRT && 
                      os_RealCompare(&argument, &zero) < 0) {
                domain_error = true;
            }
            
            if (domain_error) {
                if (result->step_count < MAX_STEPS) {
                    CalculationStep* step = &result->steps[result->step_count++];
                    
                    sprintf(step->expression, "%s domain error", func_name);
                    sprintf(step->operation, "Error");
                    sprintf(step->result, "Undefined");
                }
                return zero;
            }
            
            real_t operation_result = evaluate_function(node->function.func_type, argument);
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the function evaluation step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char arg_str[MAX_TOKEN_LENGTH];
                format_real(argument, arg_str);
                
                sprintf(step->expression, "%s(%s)", func_name, arg_str);
                sprintf(step->operation, "Calculate %s of %s", func_name, arg_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_FACTORIAL: {
            real_t expression_value = evaluate_with_steps(node->factorial.expression, result);
            
            // Check if value is a non-negative integer
            real_t rounded = os_RealRoundInt(&expression_value);
            real_t zero = os_Int24ToReal(0);
            
            if (os_RealCompare(&expression_value, &zero) < 0 || 
                os_RealCompare(&expression_value, &rounded) != 0) {
                if (result->step_count < MAX_STEPS) {
                    CalculationStep* step = &result->steps[result->step_count++];
                    
                    sprintf(step->expression, "Factorial domain error");
                    sprintf(step->operation, "Error");
                    sprintf(step->result, "Undefined");
                }
                return zero;
            }
            
            int24_t n = os_RealToInt24(&expression_value);
            real_t operation_result = os_Int24ToReal(1);
            
            for (int i = 2; i <= n; i++) {
                real_t i_real = os_Int24ToReal(i);
                operation_result = os_RealMul(&operation_result, &i_real);
            }
            
            real_t formatted_result = apply_arithmetic_format(operation_result);
            
            if (result->step_count < MAX_STEPS) {
                // Record the factorial step
                CalculationStep* step = &result->steps[result->step_count++];
                
                char expr_str[MAX_TOKEN_LENGTH];
                format_real(expression_value, expr_str);
                
                sprintf(step->expression, "%s!", expr_str);
                sprintf(step->operation, "Calculate factorial of %s", expr_str);
                format_real(formatted_result, step->result);
            }
            
            return formatted_result;
        }
        
        case NODE_PARENTHESIS: {
            // Evaluate the expression inside the parentheses
            real_t value = evaluate_with_steps(node->parenthesis.expression, result);
            
            // We don't add a separate step for parentheses
            return value;
        }
        
        default:
            // Should never happen
            return ZERO;
    }
}

/*
 *  ___             _   _            ___          _           _   _          
 * | __|  _ _ _  __| |_(_)___ _ _   | __|_ ____ _| |_  _ __ _| |_(_)___ _ _  
 * | _| || | ' \/ _|  _| / _ \ ' \  | _|\ V / _` | | || / _` |  _| / _ \ ' \ 
 * |_| \_,_|_||_\__|\__|_\___/_||_| |___|\_/\__,_|_|\_,_\__,_|\__|_\___/_||_|
 *                                                                           
 * The following functions are used to evaluate mathematical functions.
 */

/**
 * Retrieves the name of a mathematical function based on its type.
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

/**
 * Evaluates a mathematical function.
 * 
 * @param func_type The type of the function to evaluate.
 * @param argument The argument to the function.
 * @return The result of the function evaluation.
 */
static real_t evaluate_function(FunctionType func_type, real_t argument) {
    switch (func_type) {
        case FUNC_SIN:
            return os_RealSinRad(&argument);
        
        case FUNC_COS:
            return os_RealCosRad(&argument);
        
        case FUNC_TAN:
            return os_RealTanRad(&argument);
        
        case FUNC_LOG: {
            // Check for domain error
            real_t zero = os_Int24ToReal(0);
            if (os_RealCompare(&argument, &zero) <= 0) {
                // Should report an error
                return zero;
            }
            
            // Convert ln(x) to log10(x) by dividing by ln(10)
            real_t ln_result = os_RealLog(&argument);
            real_t ten = os_Int24ToReal(10);
            real_t ln_ten = os_RealLog(&ten);
            return os_RealDiv(&ln_result, &ln_ten);
        }
        
        case FUNC_LN: {
            // Check for domain error
            real_t zero = os_Int24ToReal(0);
            if (os_RealCompare(&argument, &zero) <= 0) {
                // Should report an error
                return zero;
            }
            return os_RealLog(&argument);
        }
        
        case FUNC_SQRT: {
            // Check for domain error
            real_t zero = os_Int24ToReal(0);
            if (os_RealCompare(&argument, &zero) < 0) {
                // Should report an error
                return zero;
            }
            return os_RealSqrt(&argument);
        }
        
        default:
            return ZERO;
    }
}
