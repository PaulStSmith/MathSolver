#include <tice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ti/real.h>
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
 * Sets the precision for calculations.
 * 
 * @param precision The precision setting.
 */
void set_precision(int precision) {
    current_precision = precision;
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
 * Creates a real_t representing the power of 10
 * 
 * @param power The power of 10 to create
 * @return A real_t representing 10^power
 */
static real_t power_of_10(int power) {
    real_t ten = os_Int24ToReal(10);
    real_t result = os_Int24ToReal(1);
    
    // Handle negative power
    bool is_negative = power < 0;
    power = abs(power);
    
    // Calculate 10^|power|
    for (int i = 0; i < power; i++) {
        result = os_RealMul(&result, &ten);
    }
    
    // Invert if negative power
    if (is_negative) {
        result = os_RealInv(&result);
    }
    
    return result;
}

/**
 * Truncates a value to a specific number of decimal places.
 * 
 * @param value The value to truncate.
 * @param decimal_places The number of decimal places to keep.
 * @return The truncated value.
 */
static real_t truncate_to_decimal_places(real_t value, int decimal_places) {
    if (decimal_places < 0) {
        decimal_places = 0;
    }
    
    // Create multiplier (10^decimal_places)
    real_t multiplier = power_of_10(decimal_places);
    
    // Multiply value by multiplier
    real_t scaled = os_RealMul(&value, &multiplier);
    
    // Take the integer part
    real_t int_part = os_RealInt(&scaled);
    
    // Divide by multiplier to get the truncated value
    return os_RealDiv(&int_part, &multiplier);
}

/**
 * Determines the order of magnitude of a real_t value
 * 
 * @param value The value to analyze
 * @return The order of magnitude as an integer
 */
static int get_order_of_magnitude(real_t value) {
    // Handle zero value
    real_t zero = os_Int24ToReal(0);
    if (os_RealCompare(&value, &zero) == 0) {
        return 0;
    }
    
    // Get absolute value
    real_t abs_value = value;
    if (value.sign != 0) {
        abs_value = os_RealNeg(&value);
    }
    
    // Calculate log10 of the value
    real_t ten = os_Int24ToReal(10);
    real_t log_value = os_RealLog(&abs_value);
    real_t log_ten = os_RealLog(&ten);
    real_t log10_value = os_RealDiv(&log_value, &log_ten);
    
    // Convert to integer and return
    int order = os_RealToInt24(&log10_value);
    
    return order;
}

/**
 * Truncates a value to a specific number of significant digits.
 * 
 * @param value The value to truncate.
 * @param sig_digits The number of significant digits to keep.
 * @return The truncated value.
 */
static real_t truncate_to_significant_digits(real_t value, int sig_digits) {
    log_message("Truncating to %d significant digits", sig_digits);
    
    if (sig_digits <= 0) {
        sig_digits = 1;
    }

    // Check if value is zero
    if (os_RealCompare(&value, &ZERO) == 0) {
        return ZERO;
    }
    
    // Get order of magnitude
    real_t absValue = os_RealAbs(&value);
    log_message("Absolute value: %f", os_RealToFloat(&absValue));
    real_t order = os_RealLog10(&absValue);
    log_message("Log10: %f", os_RealToFloat(&order));

    real_t floor_order = os_RealFloor(&order);
    log_message("Floor order: %f", os_RealToFloat(&floor_order));

    int order_int = os_RealToInt24(&floor_order);
    log_message("Order of magnitude: %d", order_int);

    real_t magnitute = power_of_10(order_int);
    log_message("Magnitude: %f", os_RealToFloat(&magnitute));

    // Normalize value
    real_t scaled_value = os_RealDiv(&value, &magnitute);
    log_message("Scaled value: %f", os_RealToFloat(&scaled_value));
    
    // Scale value to the correct number of significant digits
    real_t multiplier = power_of_10(sig_digits - 1);
    log_message("Multiplier: %f", os_RealToFloat(&multiplier));

    real_t truncated_value = os_RealMul(&scaled_value, &multiplier);
    log_message("Scale up: %f", os_RealToFloat(&truncated_value));

    if (scaled_value.sign < 0) 
    {
        truncated_value = os_RealCeil(&truncated_value);
        log_message("Ceil: %f", os_RealToFloat(&truncated_value));
    }
    else
    {
        truncated_value = os_RealFloor(&truncated_value);
        log_message("Floor: %f", os_RealToFloat(&truncated_value));
    }

    truncated_value = os_RealDiv(&truncated_value, &multiplier);
    log_message("Scaled down: %f", os_RealToFloat(&truncated_value));

    truncated_value = os_RealMul(&truncated_value, &magnitute);
    log_message("Final value: %f", os_RealToFloat(&truncated_value));

    return truncated_value;
}

/**
 * Rounds a value to a specific number of significant digits.
 * 
 * @param value The value to round.
 * @param sig_digits The number of significant digits to keep.
 * @return The rounded value.
 */
static real_t round_to_significant_digits(real_t value, int sig_digits) {
    if (sig_digits <= 0) {
        sig_digits = 1;
    }
    
    // Check if value is zero
    real_t zero = os_Int24ToReal(0);
    if (os_RealCompare(&value, &zero) == 0) {
        return zero;
    }
    
    // Get order of magnitude
    int order = get_order_of_magnitude(value);
    
    // Calculate decimal places needed for this many significant digits
    int decimal_places = sig_digits - order - 1;
    
    // Round to the calculated number of decimal places
    return os_RealRound(&value, decimal_places);
}

/**
 * Applies arithmetic formatting to a value.
 * 
 * @param value The value to format.
 * @return The formatted value.
 */
real_t apply_arithmetic_format(real_t value) {
    switch (current_arithmetic_type) {
        case ARITHMETIC_NORMAL:
            return value;
            
        case ARITHMETIC_TRUNCATE:
            if (current_use_significant_digits) {
                return truncate_to_significant_digits(value, current_precision);
            } else {
                return truncate_to_decimal_places(value, current_precision);
            }
            
        case ARITHMETIC_ROUND:
            if (current_use_significant_digits) {
                return round_to_significant_digits(value, current_precision);
            } else {
                return os_RealRound(&value, current_precision);
            }
            
        default:
            return value;
    }
}

/**
 * Formats a real_t number as a string.
 * 
 * @param value The value to format.
 * @param buffer The buffer to store the formatted string.
 */
void format_real(real_t value, char* buffer) {
    // Use TI's built-in real to string conversion
    int mode = 0; // Use current mode
    int8_t digits = -1; // Float mode
    
    if (current_arithmetic_type != ARITHMETIC_NORMAL) {
        mode = 1; // Normal mode
        if (current_use_significant_digits) {
            digits = -1; // Float mode
        } else {
            digits = current_precision; // Fix mode
        }
    }
    
    os_RealToStr(buffer, &value, MAX_TOKEN_LENGTH, mode, digits);
    
    // Remove trailing zeros if in float mode
    if (digits == -1) {
        int len = strlen(buffer);
        char* decimal = strchr(buffer, '.');
        
        if (decimal != NULL) {
            // Start from the end and remove trailing zeros
            while (len > 0 && buffer[len-1] == '0') {
                buffer[--len] = '\0';
            }
            
            // Remove decimal point if it's the last character
            if (len > 0 && buffer[len-1] == '.') {
                buffer[--len] = '\0';
            }
        }
    }
}

static real_t os_RealLog10(real_t* x) {
    real_t log_x = os_RealLog(x);
    return os_RealDiv(&log_x, &LOG10);
}

static real_t os_RealAbs(real_t* x) {
    if (x->sign == 0) {
        return *x;
    } else {
        return os_RealNeg(x);
    }
}

static real_t os_RealCeil(real_t* x) {
    real_t ONE = os_Int24ToReal(1);
    real_t int_part = os_RealInt(x);
    if (os_RealCompare(x, &int_part) > 0) {
        return os_RealAdd(&int_part, &ONE);
    } else {
        return int_part;
    }
}
