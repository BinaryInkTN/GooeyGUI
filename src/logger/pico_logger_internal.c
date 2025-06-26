#include "logger/pico_logger_internal.h"
#include "user_config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#if (TFT_ESPI_ENABLED == 0)

#ifndef _WIN32
#include <execinfo.h>
#endif
#include <unistd.h>

static bool logging_enabled = true;
static DebugLevel min_log_level = DEBUG_LEVEL_INFO;
static struct timespec start_time = {0};

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
    /*
    void *buffer[10];
    int size = backtrace(buffer, 10);
    char **symbols = backtrace_symbols(buffer, size);

    printf("\nStack trace:\n");
    for (int i = 0; i < size; i++)
    {
        printf("%s\n", symbols[i]);
    }
    free(symbols);
    */
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

void log_performance(char *message)
{
    if (message)
    {
        if (start_time.tv_nsec || start_time.tv_sec)
        {
            struct timespec end;

            time_t raw_time;
            struct tm *time_info;
            char time_buffer[20];
            time(&raw_time);
            time_info = localtime(&raw_time);
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

            clock_gettime(CLOCK_MONOTONIC, &end);

            double time_taken = (end.tv_sec - start_time.tv_sec) +
                                (end.tv_nsec - start_time.tv_nsec) / 1e9;

            char full_log[1024];
            printf("[%s] %sMETRICS%s Function %s took %.9f seconds to execute.\n", time_buffer, KBLU, KNRM, message, time_taken);

            snprintf(full_log, sizeof(full_log), "[%s] METRICS Function %s took %.9f seconds to execute.\n", time_buffer, message, time_taken);
            add_log_entry(full_log);
            return;
        }
        LOG_ERROR("Start time not defined.");
    }
    else
    {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
    }
}


#endif