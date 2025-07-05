#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "theme/gooey_theme.h"
#include "logger/pico_logger_internal.h"

GooeyTheme parser_load_theme_from_file(const char *filePath, bool *is_theme_loaded)
{
    *is_theme_loaded = true;

    FILE *fp = fopen(filePath, "r");
    if (fp == NULL)
    {
        printf("Error: Unable to open JSON theme file.\n");
        *is_theme_loaded = false;
        return (GooeyTheme){0};
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        fclose(fp);
        *is_theme_loaded = false;
        return (GooeyTheme){0};
    }

    fread(buffer, 1, fileSize, fp);
    buffer[fileSize] = '\0';
    fclose(fp);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    if (json == NULL)
    {
        printf("Error: Failed to parse JSON.\n");
        *is_theme_loaded = false;
        return (GooeyTheme){0};
    }

    GooeyTheme theme = {0};

    theme.base = 0x000000;
    theme.neutral = 0x808080;
    theme.widget_base = 0xFFFFFF;
    theme.primary = 0x0000FF;
    theme.danger = 0xFF0000;
    theme.info = 0x00FFFF;
    theme.success = 0x00FF00;

#define READ_COLOR(key, field)                                               \
    do                                                                       \
    {                                                                        \
        cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);           \
        if (cJSON_IsString(item) && item->valuestring != NULL)               \
        {                                                                    \
            theme.field = (unsigned long)strtol(item->valuestring, NULL, 0); \
        }                                                                    \
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


GooeyTheme parser_load_theme_from_string(const char *styling, bool *is_theme_loaded)
{
    *is_theme_loaded = true;

 

    cJSON *json = cJSON_Parse(styling);
    
    if (json == NULL)
    {
        printf("Error: Failed to parse JSON.\n");
        *is_theme_loaded = false;
        return (GooeyTheme){0};
    }

    GooeyTheme theme = {0};

    theme.base = 0x000000;
    theme.neutral = 0x808080;
    theme.widget_base = 0xFFFFFF;
    theme.primary = 0x0000FF;
    theme.danger = 0xFF0000;
    theme.info = 0x00FFFF;
    theme.success = 0x00FF00;

#define READ_COLOR(key, field)                                               \
    do                                                                       \
    {                                                                        \
        cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);           \
        if (cJSON_IsString(item) && item->valuestring != NULL)               \
        {                                                                    \
            theme.field = (unsigned long)strtol(item->valuestring, NULL, 0); \
        }                                                                    \
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

