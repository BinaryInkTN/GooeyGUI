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

#ifndef GOOEY_THEME_INTERNAL_H
#define GOOEY_THEME_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Structure representing a theme for the Gooey UI.
 *
 * The colors in the theme are stored as unsigned long values representing
 * the color codes (typically in hexadecimal format).
 */
typedef struct
{
    unsigned long base;        /**< Base window background color */
    unsigned long neutral;     /**< Neutral color for text */
    unsigned long widget_base; /**< Base widget color */
    unsigned long primary;     /**< Primary color */
    unsigned long danger;      /**< Danger color */
    unsigned long info;        /**< Info color */
    unsigned long success;     /**< Success color */
} GooeyTheme;

/**
 * @brief Loads a theme from a JSON file.
 *
 * This function reads a JSON file, parses the colors, and returns a GooeyTheme object.
 * The file should contain color values under appropriate keys for each theme attribute.
 *
 * @param filePath The path to the theme JSON file.
 * @return The parsed GooeyTheme object.
 */
GooeyTheme parser_load_theme_from_file(const char *filePath, bool *is_theme_loaded);
#endif /* GOOEY_THEME_INTERNAL_H */
