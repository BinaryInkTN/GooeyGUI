
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

#include "utils/logger/pico_logger.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef GLPS_USE_WAYLAND

#include <execinfo.h>
static struct timespec start_time = {0};

#endif
#include <unistd.h>

static bool logging_enabled = true;
static DebugLevel min_log_level = DEBUG_LEVEL_INFO;

typedef struct LogEntry
{
    char *message;
} LogEntry;

static LogEntry *log_entries = NULL;
static size_t log_capacity = 0;
static size_t log_count = 0;

void add_log_entry(const char *log_message)
{
    if (log_count == log_capacity)
    {

        size_t new_capacity = log_capacity == 0 ? 16 : log_capacity * 2;
        LogEntry *new_entries = realloc(log_entries, new_capacity * sizeof(LogEntry));
        if (!new_entries)
        {
            perror("Failed to allocate memory for log entries");
            exit(EXIT_FAILURE);
        }
        log_entries = new_entries;
        log_capacity = new_capacity;
    }

    log_entries[log_count].message = strdup(log_message);
    if (!log_entries[log_count].message)
    {
        perror("Failed to allocate memory for log message");
        exit(EXIT_FAILURE);
    }
    log_count++;
}

void free_log_entries()
{
    for (size_t i = 0; i < log_count; i++)
    {
        free(log_entries[i].message);
    }
    free(log_entries);
    log_entries = NULL;
    log_capacity = log_count = 0;
}

void log_message(DebugLevel level, const char *file, int line, const char *func, const char *fmt, ...)
{
    if (!logging_enabled || level < min_log_level)
    {
        return;
    }

    const char *color;
    const char *level_str;

    switch (level)
    {
    case DEBUG_LEVEL_INFO:
        level_str = "INFO";
        color = KBLU;
        break;
    case DEBUG_LEVEL_WARNING:
        level_str = "WARNING";
        color = KYEL;
        break;
    case DEBUG_LEVEL_ERROR:
        level_str = "ERROR";
        color = KRED;
        break;
    case DEBUG_LEVEL_CRITICAL:
        level_str = "CRITICAL";
        color = KMAG;
        break;
    default:
        level_str = "UNKNOWN";
        color = KNRM;
        break;
    }

    time_t raw_time;
    struct tm *time_info;
    char time_buffer[20];
    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

    char log_line[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_line, sizeof(log_line), fmt, args);
    va_end(args);

    printf("[%s] %s%s%s [%s:%d] %s: %s\n", time_buffer, color, level_str, KNRM, file, line, func, log_line);

    char full_log[1024];
    snprintf(full_log, sizeof(full_log), "[%s] %s [%s:%d] %s: %s", time_buffer, level_str, file, line, func, log_line);
    add_log_entry(full_log);
}

void set_logging_enabled(bool enabled)
{
    logging_enabled = enabled;
}

void set_minimum_log_level(DebugLevel level)
{
    min_log_level = level;
}

void save_log_file(const char *path)
{
    FILE *fp = fopen(path, "w");
    if (!fp)
    {
        perror("Failed to open log file");
        return;
    }

    for (size_t i = 0; i < log_count; i++)
    {
        fprintf(fp, "%s\n", log_entries[i].message);
    }

    fclose(fp);
}

void print_stack_trace(void)
{
    #ifdef GLPS_USE_WAYLAND
    void *buffer[10];
    int size = backtrace(buffer, 10);
    char **symbols = backtrace_symbols(buffer, size);

    printf("\nStack trace:\n");
    for (int i = 0; i < size; i++)
    {
        printf("%s\n", symbols[i]);
    }
    free(symbols);
    #endif
}

void dump_memory(const char *label, const void *buffer, size_t size)
{
    printf("\nMemory dump (%s):\n", label);
    unsigned char *byte_buffer = (unsigned char *)buffer;
    for (size_t i = 0; i < size; i++)
    {
        printf("%02x ", byte_buffer[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    if (size % 16 != 0)
    {
        printf("\n");
    }
}



#ifdef GLPS_USE_WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef GLPS_USE_WIN32
static LARGE_INTEGER start_time;
static LARGE_INTEGER frequency;
#else
static struct timespec start_time;
#endif

void log_performance(char *message)
{
    if (message)
    {
        #ifdef GLPS_USE_WIN32
        if (start_time.QuadPart)
        #else
        if (start_time.tv_nsec || start_time.tv_sec)
        #endif
        {
            char time_buffer[20];
            time_t raw_time;
            struct tm *time_info;
            time(&raw_time);
            time_info = localtime(&raw_time);
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

            double time_taken;

            #ifdef GLPS_USE_WIN32
            LARGE_INTEGER end;
            QueryPerformanceCounter(&end);
            time_taken = (double)(end.QuadPart - start_time.QuadPart) / frequency.QuadPart;
            #else
            struct timespec end;
            clock_gettime(CLOCK_MONOTONIC, &end);
            time_taken = (end.tv_sec - start_time.tv_sec) +
                         (end.tv_nsec - start_time.tv_nsec) / 1e9;
            #endif

            char full_log[1024];
            printf("[%s] METRICS Function %s took %.9f seconds to execute.\n", time_buffer, message, time_taken);
            snprintf(full_log, sizeof(full_log), "[%s] METRICS Function %s took %.9f seconds to execute.\n", time_buffer, message, time_taken);
            add_log_entry(full_log);
            return;
        }
        LOG_ERROR("Start time not defined.");
    }
    else
    {
        #ifdef GLPS_USE_WIN32
        QueryPerformanceCounter(&start_time);
        QueryPerformanceFrequency(&frequency);
        #else
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        #endif
    }
}
