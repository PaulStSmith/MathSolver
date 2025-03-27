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
#include "headers/ui.h"
#include "headers/kb_handler.h"
#include "headers/log.h"
#include "headers/log_private.h"

/* ============================== Logger Variables ============================== */

/** Maximum length of a single log message */
#define MAX_LOG_MSG_LENGTH 100

/** Debug AppVar name */
#define DEBUG_APPVAR_NAME "DBGLOG"

/** Debug buffer to prepare log messages */
static char debug_buffer[MAX_LOG_MSG_LENGTH];

/**
 * Initializes the debug logger.
 * Creates the debug AppVar if it doesn't exist.
 */
void logger_init(void) {
    #ifndef DEBUG
    return;
    #endif

   clear_screen();
   println("Initializing logger");

    // Clear the log
    uint8_t handle = logger_get_handle("w");
    if (handle) {
       // Write initial message
       const char *init_msg = "\nMathSolver Debug Log\n";
       ti_Write(init_msg, 1, strlen(init_msg), handle);
       ti_Close(handle);
       ti_SetArchiveStatus(true, handle);
   }

   println("Logger initialized");
}

/**
 * Closes the debug logger.
 * Appends a closing message to the log.
 */
void logger_close(void) {
    #ifndef DEBUG
    return;
    #endif

    // Append the log
    uint8_t handle = logger_get_handle("a");
    if (!handle) {
        return;
    }
    
    // Append log message
    const char *close_msg = "\nMathSolver Debug Log Closed\n";
    ti_Write(close_msg, 1, strlen(close_msg), handle);
    ti_Close(handle);
    ti_SetArchiveStatus(true, handle);
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
    vsnprintf(debug_buffer, MAX_LOG_MSG_LENGTH - 2, format, args);
    va_end(args);
    
    // Add newline
    strcat(debug_buffer, "\n");

    dbg_printf("%s", debug_buffer);
    
    // Append the log
    uint8_t handle = logger_get_handle("a");
    if (!handle) {
        return;
    }
    
    // Append log message
    ti_Write(debug_buffer, 1, strlen(debug_buffer), handle);
    
    // Close AppVar
    ti_Close(handle);
}

/**
 * Gets the handle to the debug AppVar.
 * 
 * @param mode The mode to open the AppVar in.
 * @return The handle to the AppVar.
 */
static uint8_t logger_get_handle(char* mode) {
    uint8_t handle = ti_Open(DEBUG_APPVAR_NAME, mode);
    if (!handle) {
        // Couldn't open AppVar, try creating it
        handle = ti_Open(DEBUG_APPVAR_NAME, "w");
        if (!handle) {
            // Still couldn't open, give up
            return 0;
        }
    }
    return handle;
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
