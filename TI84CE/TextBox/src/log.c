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
#define MAX_LOG_MSG_LENGTH 100

/** Debug AppVar name */
#define DEBUG_APPVAR_NAME "DBGLOG"

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
 * Logs a debug message.
 * 
 * @param message The debug message to log.
 */
void log_debug(const char* message) {
    log_message("DEBUG: %s", message);
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
 * Logs an error message.
 * 
 * @param error The error message.
 */
void log_error(const char* error) {
    log_message("ERROR: %s", error);
}
