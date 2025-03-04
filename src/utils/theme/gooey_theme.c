/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "utils/theme/gooey_theme_internal.h"

GooeyTheme parser_load_theme_from_file(const char *filePath)
{
    FILE *fp = fopen(filePath, "r");
    if (fp == NULL)
    {
        printf("Error: Unable to open json theme file.\n");
        exit(1);
    }

    char buffer[1024];
    int len = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        exit(1);
    }

    int baseColor, widgetBaseColor, neutralColor, primaryColor, dangerColor, infoColor, successColor;
    // Neutral color
    cJSON *neutral = cJSON_GetObjectItemCaseSensitive(json, "neutral");
    if (cJSON_IsString(neutral) && (neutral->valuestring != NULL))
    {
        neutralColor = (unsigned long)strtol(neutral->valuestring, NULL, 0);
    }

    // Danger color
    cJSON *danger = cJSON_GetObjectItemCaseSensitive(json, "danger");
    if (cJSON_IsString(danger) && (danger->valuestring != NULL))
    {
        dangerColor = (unsigned long)strtol(danger->valuestring, NULL, 0);
    }

    // Info color
    cJSON *info = cJSON_GetObjectItemCaseSensitive(json, "info");
    if (cJSON_IsString(info) && (info->valuestring != NULL))
    {
        infoColor = (unsigned long)strtol(info->valuestring, NULL, 0);
    }

    // Success color
    cJSON *success = cJSON_GetObjectItemCaseSensitive(json, "success");
    if (cJSON_IsString(success) && (success->valuestring != NULL))
    {
        successColor = (unsigned long)strtol(success->valuestring, NULL, 0);
    }

    // Widget Base Color

    cJSON *widget_base = cJSON_GetObjectItemCaseSensitive(json, "widget_base");
    if (cJSON_IsString(widget_base) && (widget_base->valuestring != NULL))
    {
        widgetBaseColor = (unsigned long)strtol(widget_base->valuestring, NULL, 0);
    }

    // Base color
    cJSON *base = cJSON_GetObjectItemCaseSensitive(json, "base");
    if (cJSON_IsString(base) && (base->valuestring != NULL))
    {
        baseColor = (unsigned long)strtol(base->valuestring, NULL, 0);
    }

    // Primary color
    cJSON *primary = cJSON_GetObjectItemCaseSensitive(json, "primary");
    if (cJSON_IsString(primary) && (primary->valuestring != NULL))
    {
        primaryColor = (unsigned long)strtol(primary->valuestring, NULL, 0);
    }

    cJSON_Delete(json);

    GooeyTheme theme = {.base = baseColor, .neutral = neutralColor, .widget_base = widgetBaseColor, .primary = primaryColor, .danger = dangerColor, .info = infoColor, .success = successColor};

    return theme;
}
