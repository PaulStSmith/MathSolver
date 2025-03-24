/**
 * Debug Logger Using fileIOC for MathSolver TI-84 CE
 * 
 * This implementation uses TI's fileIOC library to store debug logs
 * in an AppVar, making them persistent and accessible outside the app.
 */

 #include <string.h>
 #include <stdio.h>
 #include <stdarg.h>
 #include <stdbool.h>
 #include <fileioc.h>
 #include "headers/ui.h"
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

    clear_screen();
    println("Initializing logger");

     // Try to open the debug AppVar
     uint8_t handle = ti_Open(DEBUG_APPVAR_NAME, "w");
     
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
  * Adds a message to the debug log.
  * 
  * @param format Format string similar to printf.
  * @param ... Variable arguments for the format string.
  */
 void log_message(const char* format, ...) {
     // Format the message
     va_list args;
     va_start(args, format);
     vsnprintf(debug_buffer, MAX_LOG_MSG_LENGTH - 2, format, args);
     va_end(args);
     
     // Add newline
     strcat(debug_buffer, "\n");
     
     // Open the AppVar in append mode
     uint8_t handle = ti_Open(DEBUG_APPVAR_NAME, "a");
     if (!handle) {
         // Couldn't open AppVar, try creating it
         handle = ti_Open(DEBUG_APPVAR_NAME, "w");
         if (!handle) {
             // Still couldn't open, give up
             return;
         }
     }
     
     // Append log message
     ti_Write(debug_buffer, 1, strlen(debug_buffer), handle);
     
     // Close AppVar
     ti_Close(handle);
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

 void log_token(const char* source, int type, const char* value) {
    if (type == TOKEN_NONE) {
        log_message("(%s) Token not identified: '%s' (0x%x)", source, value, value);
    }
    else
        log_message("(%s) Token identified: Type=%s, Value=%s", source, get_token_type(type), value);
 }
 
 void log_token_char(const char* source, int type, const char value) {
    if (type == TOKEN_NONE) {
        log_message("(%s) Token not identified: '%s' (0x%x)", source, value, value);
    }
    else
        log_message("(%s) Token identified: Type=%s, Value=%s", source, get_token_type(type), value);
 }
 
 /**
  * Clears the debug log.
  */
 void clear_debug_log(void) {
     // Open the AppVar in write mode (this deletes previous content)
     uint8_t handle = ti_Open(DEBUG_APPVAR_NAME, "w");
     if (handle) {
         // Write initial message
         const char *init_msg = "MathSolver Debug Log - Cleared\n";
         ti_Write(init_msg, 1, strlen(init_msg), handle);
         ti_Close(handle);
     }
 }
 
 /**
  * Gets the contents of the debug log.
  * 
  * @param buffer Buffer to store the log.
  * @param max_size Maximum size of the buffer.
  * @param offset Offset to start reading from (0 for beginning).
  * @return Number of bytes read, or -1 on error.
  */
 int get_debug_log(char* buffer, int max_size, int offset) {
     if (!buffer || max_size <= 0) return -1;
     
     // Open the AppVar for reading
     uint8_t handle = ti_Open(DEBUG_APPVAR_NAME, "r");
     if (!handle) {
         // AppVar doesn't exist
         buffer[0] = '\0';
         return 0;
     }
     
     // Set position to offset
     if (offset > 0) {
         if (ti_Seek(offset, SEEK_SET, handle) == EOF) {
             ti_Close(handle);
             buffer[0] = '\0';
             return 0;
         }
     }
     
     // Read data into buffer
     int bytes_read = ti_Read(buffer, 1, max_size - 1, handle);
     
     // Null-terminate the buffer
     buffer[bytes_read] = '\0';
     
     // Close AppVar
     ti_Close(handle);
     
     return bytes_read;
 }
 
 /**
  * Gets the total size of the debug log.
  * 
  * @return The size of the debug log in bytes, or 0 if AppVar doesn't exist.
  */
 int get_debug_log_size(void) {
     // Open the AppVar for reading
     uint8_t handle = ti_Open(DEBUG_APPVAR_NAME, "r");
     if (!handle) {
         // AppVar doesn't exist
         return 0;
     }
     
     // Get size
     int size = ti_GetSize(handle);
     
     // Close AppVar
     ti_Close(handle);
     
     return size;
 }

 static char* get_token_type(int type) {
    switch (type)
    {
    case TOKEN_COMMA:       return "COMMA";
    case TOKEN_DIVIDE:      return "DIVIDE";
    case TOKEN_END:         return "END";
    case TOKEN_FACTORIAL:   return "FACTORIAL";
    case TOKEN_FUNCTION:    return "FUNCTION";
    case TOKEN_LEFT_PAREN:  return "LEFT_PAREN";
    case TOKEN_MINUS:       return "MINUS";
    case TOKEN_MULTIPLY:    return "MULTIPLY";
    case TOKEN_NONE:        return "NONE";
    case TOKEN_NUMBER:      return "NUMBER";
    case TOKEN_PLUS:        return "PLUS";
    case TOKEN_POWER:       return "POWER";
    case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
    case TOKEN_VARIABLE:    return "VARIABLE";
    case TOKEN_PI:          return "PI";
    case TOKEN_PHI:         return "PHI";
    default:
        return "-";
    }
}
