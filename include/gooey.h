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

#ifndef GOOEY_H
#define GOOEY_H


#include "gooey_widgets_internal.h"
#include "core/gooey_backend_internal.h"


#include "widgets/gooey_button.h"
#include "widgets/gooey_canvas.h"
#include "widgets/gooey_checkbox.h"
#include "widgets/gooey_dropdown.h"
#include "widgets/gooey_label.h"
#include "widgets/gooey_layout.h"
#include "widgets/gooey_list.h"
#include "widgets/gooey_menu.h"
#include "widgets/gooey_messagebox.h"
#include "widgets/gooey_radiobutton.h"
#include "widgets/gooey_slider.h"
#include "widgets/gooey_textbox.h"
#include "widgets/gooey_plot.h"
#include "signals/gooey_signals.h"

/**
 * @brief Initializes the Gooey system with the specified backend.
 *
 * @param backend The backend to use (e.g., X11, Wayland, GLFW).
 * @return 0 on success, non-zero on failure.
 */
int Gooey_Init(GooeyBackends backend);

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


#endif
