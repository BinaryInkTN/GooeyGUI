/*
 Copyright (c) 2025 Yassine Ahmed Ali

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * @file pico_logger.h
 * @brief Provides logging and debugging utilities with configurable log levels.
 */

#ifndef PICO_LOGGER_H
#define PICO_LOGGER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief ANSI color codes for colored terminal output.
 *
 * These macros define escape sequences used to set text colors in terminal output.
 * They are primarily used for highlighting log messages based on severity.
 */
#define KNRM "\x1B[0m"  /**< Reset color */
#define KRED "\x1B[31m" /**< Red color for errors */
#define KGRN "\x1B[32m" /**< Green color for success */
#define KYEL "\x1B[33m" /**< Yellow color for warnings */
#define KBLU "\x1B[34m" /**< Blue color for informational messages */
#define KMAG "\x1B[35m" /**< Magenta color for special messages */
#define KCYN "\x1B[36m" /**< Cyan color for debug messages */
#define KWHT "\x1B[37m" /**< White color for general text */

/**
 * @enum DebugLevel
 * @brief Log levels used for message categorization.
 *
 * The log levels allow messages to be classified by severity, ranging from
 * general informational messages to critical errors.
 */
typedef enum
{
    DEBUG_LEVEL_INFO,    /**< Informational messages */
    DEBUG_LEVEL_WARNING, /**< Warnings indicating potential issues */
    DEBUG_LEVEL_ERROR,   /**< Error messages indicating a problem */
    DEBUG_LEVEL_CRITICAL /**< Critical error messages indicating a failure */
} DebugLevel;

/**
 * @brief Macro to log a message with the specified log level.
 *
 * This macro logs messages at various levels, such as informational, warning,
 * error, or critical. It includes the file name, line number, and function name
 * where the log was generated.
 *
 * @param level The log level (e.g., DEBUG_LEVEL_INFO).
 * @param fmt The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
#define LOG_MESSAGE(level, fmt, ...) log_message(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/**
 * @brief Macro to log an informational message.
 *
 * This macro logs general informational messages using the `DEBUG_LEVEL_INFO` log level.
 *
 * @param fmt The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
#define LOG_INFO(fmt, ...) LOG_MESSAGE(DEBUG_LEVEL_INFO, fmt, ##__VA_ARGS__)

/**
 * @brief Macro to log a warning message.
 *
 * This macro logs warning messages indicating potential issues using the `DEBUG_LEVEL_WARNING` log level.
 *
 * @param fmt The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
#define LOG_WARNING(fmt, ...) LOG_MESSAGE(DEBUG_LEVEL_WARNING, fmt, ##__VA_ARGS__)

/**
 * @brief Macro to log an error message.
 *
 * This macro logs error messages indicating a problem using the `DEBUG_LEVEL_ERROR` log level.
 *
 * @param fmt The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
#define LOG_ERROR(fmt, ...) LOG_MESSAGE(DEBUG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

/**
 * @brief Macro to log a critical error message.
 *
 * This macro logs critical error messages indicating severe issues that may cause application failure.
 *
 * @param fmt The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
#define LOG_CRITICAL(fmt, ...) LOG_MESSAGE(DEBUG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

/**
 * @brief Logs a message with detailed information.
 *
 * This function logs a formatted message with details about the file, line number,
 * and function where the log was generated.
 *
 * @param level The log level (e.g., DEBUG_LEVEL_INFO).
 * @param file The source file where the log was generated.
 * @param line The line number in the source file.
 * @param func The function name where the log was generated.
 * @param fmt The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
void log_message(DebugLevel level, const char *file, int line, const char *func, const char *fmt, ...);

/**
 * @brief Logs a performance metric message.
 *
 * This function logs a performance-related message, useful for timing and profiling.
 *
 * @param message The performance-related message to log.
 */
void log_performance(char *message);

/**
 * @brief Macro to log a performance-related message.
 *
 * This macro wraps the `log_performance` function for easier usage.
 *
 * @param message The performance-related message to log.
 */
#define LOG_PERFORMANCE(message) log_performance(message)

/**
 * @brief Enables or disables logging.
 *
 * This function controls whether logging is active. If disabled, no log messages
 * will be generated.
 *
 * @param enabled If true, logging is enabled; if false, logging is disabled.
 */
void set_logging_enabled(bool enabled);

/**
 * @brief Sets the minimum log level for logging.
 *
 * This function defines the minimum log level for messages to be logged. Messages
 * below the specified level will not be logged.
 *
 * @param level The minimum log level (e.g., DEBUG_LEVEL_WARNING).
 */
void set_minimum_log_level(DebugLevel level);

/**
 * @brief Prints the current stack trace.
 *
 * This function outputs the current call stack to aid in debugging and tracing
 * the program's execution flow.
 */
void print_stack_trace(void);

/**
 * @brief Dumps the memory contents of a buffer.
 *
 * This function outputs the contents of a memory buffer in a human-readable
 * hexadecimal format, useful for debugging low-level memory issues.
 *
 * @param label A label describing the memory buffer.
 * @param buffer A pointer to the memory buffer to be dumped.
 * @param size The size of the memory buffer in bytes.
 */
void dump_memory(const char *label, const void *buffer, size_t size);

/**
 * @brief Saves the logged messages to a file.
 *
 * This function writes all the accumulated log messages to a specified file.
 * Each log message is written to a new line in the file. If the file cannot be
 * opened, an error is printed to the standard output.
 *
 * @param path The path to the file where the log messages will be saved.
 *             If the file does not exist, it will be created. If it already
 *             exists, its contents will be overwritten.
 */
void save_log_file(const char *path);

#endif
