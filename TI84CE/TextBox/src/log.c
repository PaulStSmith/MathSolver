/**
 * Debug Logger Using fileIOC for MathSolver TI-84 CE
 * 
 * This implementation uses TI's fileIOC library to store debug logs
 * in an AppVar, making them persistent and accessible outside the app.
 */

#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fileioc.h>
#include "headers/log_private.h"

/* ============================== Logger Variables ============================== */

/** Maximum length of a single log message */
#define MAX_LOG_MSG_LENGTH 255

/**
 * Initializes the debug logger.
 * Creates the debug AppVar if it doesn't exist.
 */
void logger_init(void) {
    #ifndef DEBUG
    return;
    #endif
    dbg_ClearConsole();
    log_message("Logger initialized.");
}

/**
 * Closes the debug logger.
 * Appends a closing message to the log.
 */
void logger_close(void) {
    #ifndef DEBUG
    return;
    #endif
    log_message("Logger closed.");
}

/**
 * Adds a message to the debug log.
 * 
 * @param format Format string similar to printf.
 * @param ... Variable arguments for the format string.
 */
void log_message(const char* format, ...) {
    #ifndef DEBUG
    return;
    #endif
    
    // Format the message
    va_list args;
    va_start(args, format);
    static char msg_format[MAX_LOG_MSG_LENGTH];
    snprintf(msg_format, MAX_LOG_MSG_LENGTH - 2, "%s\n", format);

    static char msg[MAX_LOG_MSG_LENGTH];
    vsnprintf(msg, MAX_LOG_MSG_LENGTH - 2, msg_format, args);
    dbg_printf("%s", msg);

    va_end(args);    
}

/**
 * Logs an informational message.
 * 
 * @param format Format string similar to printf.
 * @param ... Variable arguments for the format string.
 */
void log_info(const char* format, ...) {
    #ifndef DEBUG
    return;
    #endif

    va_list args;
    va_start(args, format);
    print_log("INFO", format, args);
    va_end(args);
}

/**
 * Logs a warning message.
 * 
 * @param format Format string similar to printf.
 * @param ... Variable arguments for the format string.
 */
void log_warning(const char* format, ...) {
    #ifndef DEBUG
    return;
    #endif

    va_list args;
    va_start(args, format);
    print_log("WARNING", format, args);
    va_end(args);
}

/**
 * Logs a critical error message.
 * 
 * @param format Format string similar to printf.
 * @param ... Variable arguments for the format string.
 */
void log_critical(const char* format, ...) {
    #ifndef DEBUG
    return;
    #endif

    va_list args;
    va_start(args, format);
    print_log("CRITICAL", format, args);
    va_end(args);
}

/**
 * Logs a debug message.
 * 
 * @param format Format string similar to printf.
 * @param ... Variable arguments for the format string.
 */
void log_debug(const char* format, ...) {
    #ifndef DEBUG
    return;
    #endif

    va_list args;
    va_start(args, format);
    print_log("DEBUG", format, args);
    va_end(args);
}

/**
 * Logs an error message.
 * 
 * @param format Format string similar to printf.
 * @param ... Variable arguments for the format string.
 */
void log_error(const char* format, ...) {
    #ifndef DEBUG
    return;
    #endif

    va_list args;
    va_start(args, format);
    print_log("ERROR", format, args);
    va_end(args);
}

/**
 * Logs a variable value.
 * 
 * @param name Variable name.
 * @param value Variable value.
 */
void log_variable(const char* name, real_t value) {
    log_message("VAR: %s = %.6f", name, value);
}

/**
 * Logs an operation with its result.
 * 
 * @param operation The operation description.
 * @param result The result of the operation.
 */
void log_operation(const char* operation, char* result) {
    log_message("OP: %s = %.6f", operation, result);
}

/**
 * Helper function to format and print log messages.
 * 
 * @param tag The log level tag (e.g., INFO, WARNING).
 * @param format Format string similar to printf.
 * @param args Variable argument list.
 */
static void print_log(const char* tag, const char* format, va_list args) {
    static char msg[MAX_LOG_MSG_LENGTH];
    vsnprintf(msg, MAX_LOG_MSG_LENGTH - 2, format, args);
    dbg_printf("%s: %s\n", tag, msg);
}
