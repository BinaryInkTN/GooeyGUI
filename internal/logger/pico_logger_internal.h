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
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// ANSI color codes for colored terminal output
#define KNRM "\x1B[0m"  /**< Reset color */
#define KRED "\x1B[31m" /**< Red color for errors */
#define KGRN "\x1B[32m" /**< Green color for success */
#define KYEL "\x1B[33m" /**< Yellow color for warnings */
#define KBLU "\x1B[34m" /**< Blue color for informational messages */
#define KMAG "\x1B[35m" /**< Magenta color for special messages */
#define KCYN "\x1B[36m" /**< Cyan color for debug messages */
#define KWHT "\x1B[37m" /**< White color for general text */

typedef enum
{
    DEBUG_LEVEL_INFO,    /**< Informational messages */
    DEBUG_LEVEL_WARNING, /**< Warnings indicating potential issues */
    DEBUG_LEVEL_ERROR,   /**< Error messages indicating a problem */
    DEBUG_LEVEL_CRITICAL /**< Critical error messages indicating a failure */
} DebugLevel;

#if (TFT_ESPI_ENABLED==0)
#define LOG_MESSAGE(level, fmt, ...) \
    log_message(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)


#define LOG_INFO(fmt, ...)     LOG_MESSAGE(DEBUG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)  LOG_MESSAGE(DEBUG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    LOG_MESSAGE(DEBUG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOG_MESSAGE(DEBUG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

void log_message(DebugLevel level, const char *file, int line, const char *func, const char *fmt, ...);
void log_performance(char *message);

#define LOG_PERFORMANCE(message) log_performance(message)

void set_logging_enabled(bool enabled);
void set_minimum_log_level(DebugLevel level);
void print_stack_trace(void);
void dump_memory(const char *label, const void *buffer, size_t size);
void save_log_file(const char *path);

#else 
     #define log_message(level, file, line, func, fmt, ...) ((void)0)
    
    #define LOG_INFO(fmt, ...)     ((void)0)
    #define LOG_WARNING(fmt, ...)  ((void)0)
    #define LOG_ERROR(fmt, ...)    ((void)0)
    #define LOG_CRITICAL(fmt, ...) ((void)0)
    #define LOG_PERFORMANCE(msg)   ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif // PICO_LOGGER_H
