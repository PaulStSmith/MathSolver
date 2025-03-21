#include <tice.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "headers/log.h"
#include "headers/mathsolver.h"
#include "headers/arithmetic_private.h"

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
