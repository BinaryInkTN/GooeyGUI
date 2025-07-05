#ifndef GOOEY_THEME_INTERNAL_H
#define GOOEY_THEME_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

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
 * @param is_theme_loaded Output parameter set to true if the theme loaded successfully, false otherwise.
 * @return The parsed GooeyTheme object. If loading fails, the returned object contains undefined data.
 */
GooeyTheme parser_load_theme_from_file(const char *filePath, bool *is_theme_loaded);
GooeyTheme parser_load_theme_from_string(const char *styling, bool *is_theme_loaded);
#ifdef __cplusplus
}
#endif

#endif /* GOOEY_THEME_INTERNAL_H */
