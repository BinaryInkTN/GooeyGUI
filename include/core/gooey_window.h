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

#ifndef GOOEY_COMMON_H
#define GOOEY_COMMON_H

#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Loads a theme from the specified file path.
 *
 * This function reads a theme file and returns a `GooeyTheme` object that
 * can be applied to Gooey windows.
 *
 * @param theme_path The path to the theme file.
 * @return A pointer to the loaded `GooeyTheme` object.
 */
GooeyTheme *GooeyTheme_LoadFromFile(const char *theme_path);
GooeyTheme *GooeyTheme_LoadFromString(const char *styling);
/**
 * @brief Sets the theme for the specified Gooey window.
 *
 * This function applies the given theme to the window, updating its appearance.
 *
 * @param win The Gooey window to set the theme for.
 * @param theme The theme to apply to the window.
 */
void GooeyWindow_SetTheme(GooeyWindow *win, GooeyTheme *theme);

/**
 * @brief Creates a new Gooey window with the specified title, width, and height.
 *
 * This function initializes and returns a new window. The window can be customized
 * with various properties such as its visibility and theme.
 *
 * @param title The title of the window.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param visibility The initial visibility of the window (visible or hidden).
 * @return A pointer to the newly created `GooeyWindow` object.
 */
GooeyWindow *GooeyWindow_Create(const char *title, int width, int height, bool visibility);

void GooeyWindow_EnableDebugOverlay(GooeyWindow *win, bool is_enabled);

/**
 * @brief Sets the visibility of a Gooey window.
 *
 * This function controls whether the window is visible or hidden.
 *
 * @param win The window whose visibility is to be set.
 * @param visibility `true` to make the window visible, `false` to hide it.
 */
void GooeyWindow_MakeVisible(GooeyWindow *win, bool visibility);

/**
 * @brief Runs the Gooey window's event loop.
 *
 * This function starts the main event loop for the specified window, where
 * user input and window events are processed until the window is closed.
 *
 * @param num_windows The number of windows to handle in the event loop.
 * @param first_win The first window to run in the event loop.
 * @param ... Additional windows to handle in the event loop.
 */
void GooeyWindow_Run(int num_windows, GooeyWindow *first_win, ...);

void GooeyTheme_Destroy(GooeyTheme *theme);

/**
 * @brief Cleans up the resources associated with Gooey windows.
 *
 * This function deallocates memory and resources for the specified windows
 * and their associated widgets.
 *
 * @param num_windows The number of windows to clean up.
 * @param first_win The first window to clean up.
 * @param ... Additional windows to clean up.
 */
void GooeyWindow_Cleanup(int num_windows, GooeyWindow *first_win, ...);

/**
 * @brief Registers a widget with the specified window.
 *
 * This function adds a widget (such as a button, label, or slider) to a window,
 * so it can be rendered and interact with user input.
 *
 * @param win The window to register the widget with.
 * @param widget The widget to register.
 */
void GooeyWindow_RegisterWidget(GooeyWindow *win, void *widget);

/**
 * @brief Sets the resizable property of a window.
 *
 * This function determines whether the user can resize the window. If the window
 * is made resizable, the user can adjust its size at runtime.
 *
 * @param msgBoxWindow The window to modify.
 * @param is_resizable `true` to make the window resizable, `false` to make it fixed-size.
 */
void GooeyWindow_MakeResizable(GooeyWindow *msgBoxWindow, bool is_resizable);

void GooeyWindow_ToggleDecorations(GooeyWindow *win, bool enable);

/**
 * @brief Requests a redraw of the window.
 *
 * This function triggers a redraw of the specified window, typically in response
 * to changes that affect its appearance (e.g., updates to the window's content or layout).
 *
 * @param win The window that requires a redraw.
 */
void GooeyWindow_RequestRedraw(GooeyWindow *win);

void GooeyWindow_SetContinuousRedraw(GooeyWindow *win);

#ifdef __cplusplus
}
#endif

#endif /* GOOEY_COMMON_H */
