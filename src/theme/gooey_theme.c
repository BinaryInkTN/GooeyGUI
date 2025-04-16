#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "theme/gooey_theme.h"
#include "logger/pico_logger_internal.h"

GooeyTheme *parser_load_theme_from_file(const char *filePath)
{
    FILE *fp = fopen(filePath, "r");
    if (fp == NULL)
    {
        printf("Error: Unable to open JSON theme file.\n");
        return NULL;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    // Allocate memory dynamically for the file content
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        fclose(fp);
        return NULL;
    }

    fread(buffer, 1, fileSize, fp);
    buffer[fileSize] = '\0'; // Null-terminate JSON string
    fclose(fp);

    // Parse JSON
    cJSON *json = cJSON_Parse(buffer);
    free(buffer); // Free buffer after parsing

    if (json == NULL)
    {
        printf("Error: Failed to parse JSON.\n");
        return NULL;
    }

    // Initialize theme colors with defaults
    GooeyTheme *theme = calloc(1, sizeof(GooeyTheme));
    if (!theme)
    {
        printf("Error: Memory allocation for theme failed.\n");
        cJSON_Delete(json);
        return NULL;
    }

    theme->base = 0x000000; // Default black
    theme->neutral = 0x808080; // Default gray
    theme->widget_base = 0xFFFFFF; // Default white
    theme->primary = 0x0000FF; // Default blue
    theme->danger = 0xFF0000; // Default red
    theme->info = 0x00FFFF; // Default cyan
    theme->success = 0x00FF00; // Default green

    // Read colors from JSON
    #define READ_COLOR(key, field) \
        do { \
            cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key); \
            if (cJSON_IsString(item) && item->valuestring != NULL) { \
                theme->field = (unsigned long)strtol(item->valuestring, NULL, 0); \
            } \
        } while (0)

    READ_COLOR("base", base);
    READ_COLOR("neutral", neutral);
    READ_COLOR("widget_base", widget_base);
    READ_COLOR("primary", primary);
    READ_COLOR("danger", danger);
    READ_COLOR("info", info);
    READ_COLOR("success", success);

    cJSON_Delete(json);
    return theme;
}
