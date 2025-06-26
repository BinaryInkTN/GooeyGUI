/**
 * @file gooey_button_internal.h
 * @brief Internal button handling functions for the Gooey GUI library.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains internal functions for handling and drawing buttons in a Gooey window.
 */

#ifndef GOOEY_BUTTON_INTERNAL_H
#define GOOEY_BUTTON_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"
#if (ENABLE_BUTTON)
#include <stdbool.h>

/**
 * @brief Handles button click events.
 *
 * Checks if a click at the specified coordinates intersects any button
 * in the window. If so, the corresponding callback is triggered.
 *
 * @param win The window containing the button.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return `true` if a button was clicked, `false` otherwise.
 */
bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Handles button hover events.
 *
 * Checks if the mouse is hovering over any button and updates
 * the hover state for visual feedback.
 *
 * @param win The window containing the button.
 * @param x The current x-coordinate of the mouse.
 * @param y The current y-coordinate of the mouse.
 * @return `true` if hovering over a button, `false` otherwise.
 */
bool GooeyButton_HandleHover(GooeyWindow *win, int x, int y);

/**
 * @brief Draws the button on the window.
 *
 * Renders all buttons in the specified window, including their label
 * and visual states (e.g., hover or pressed).
 *
 * @param win The window to draw the buttons on.
 */
void GooeyButton_Draw(GooeyWindow *win);

#endif // ENABLE_BUTTON

#ifdef __cplusplus
}
#endif

#endif // GOOEY_BUTTON_INTERNAL_H
