#ifndef GOOEY_COMMON_H
#define GOOEY_COMMON_H

#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

#include "gooey_widgets_internal.h"
#include "utils/theme/gooey_theme_internal.h"
#include "gooey_event_internal.h"
#include "utils/logger/gooey_logger_internal.h"




/**
 * @brief Sets the theme for the Gooey window.
 *
 * @param fontPath The path to the font file to use for the window's theme.
 */
void GooeyWindow_SetTheme(const char *fontPath);

/**
 * @brief Creates a new Gooey window.
 *
 * This function initializes a new window with the given title, width, and height.
 *
 * @param title The title of the window.
 * @param width The width of the window.
 * @param height The height of the window.
 * @return A new GooeyWindow object.
 */
GooeyWindow GooeyWindow_Create(const char *title, int width, int height, bool visibility);

GooeyWindow GooeyWindow_CreateChild(const char *title, int width, int height, bool visibility);
void GooeyWindow_MakeVisible(GooeyWindow *win, bool visibility);

/**
 * @brief Runs the Gooey window.
 *
 * This function enters the main event loop and begins processing user input
 * and window events for the provided Gooey window.
 *
 * @param win The window to run.
 */
void GooeyWindow_Run(int num_windows, GooeyWindow *first_win, ...);

/**
 * @brief Cleans up the resources associated with the Gooey windows.
 * @param num_windows The number of windows to clean up.
 * @param first_window The first window to clean up.
 */
void GooeyWindow_Cleanup(int num_windows, GooeyWindow *first_win, ...);
void GooeyWindow_RegisterWidget(GooeyWindow *win, GooeyWidget *widget);
/**
 * @brief Sets the resizable property of a window.
 *
 * Allows the user to resize a window dynamically, depending on the value of `is_resizable`.
 *
 * @param msgBoxWindow A pointer to the window whose resizable property is to be set.
 * @param is_resizable `true` to make the window resizable; `false` to make it fixed-size.
 */
void GooeyWindow_MakeResizable(GooeyWindow *msgBoxWindow, bool is_resizable);

//void GooeyWindow_Redraw(GooeyWindow *win);

extern GooeyTheme *active_theme;

#endif