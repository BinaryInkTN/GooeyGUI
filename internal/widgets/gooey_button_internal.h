
/**
 * @file gooey_button.h
 * @brief Button handling functions for the Gooey GUI library.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains functions for adding, handling, and drawing buttons in a Gooey window.
 */

#ifndef GOOEY_BUTTON_INTERNAL_H
#define GOOEY_BUTTON_INTERNAL_H

#include "common/gooey_common.h"
#if (ENABLE_BUTTON)
#include <stdbool.h>

/**
 * @brief Handles button click events.
 *
 * This function checks if a click event at the specified coordinates intersects with any button
 * in the window. If a button is clicked, its callback function is triggered.
 *
 * @param win The window containing the button.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return `true` if a button was clicked, `false` otherwise.
 */
bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y);


bool GooeyButton_HandleHover(GooeyWindow *win, int x, int y);
/**
 * @brief Draws the button on the window.
 *
 * This function renders the button on the specified window, including its label and any visual states
 * (e.g., highlighted or clicked).
 *
 * @param win The window to draw the button on.
 */
void GooeyButton_Draw(GooeyWindow *win);


#endif
#endif