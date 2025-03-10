/**
 * @file gooey_button.h
 * @brief Button handling functions for the Gooey GUI library.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains functions for adding, handling, and drawing buttons in a Gooey window.
 */

#ifndef GOOEY_BUTTON_H
#define GOOEY_BUTTON_H

#include "common/gooey_common.h"
#include <stdbool.h>

/**
 * @brief Adds a button to the window.
 *
 * This function creates a new button with the specified label, position, dimensions, and callback function.
 * The button is added to the provided window.
 *
 * @param label The label to display on the button.
 * @param x The x-coordinate of the button's position.
 * @param y The y-coordinate of the button's position.
 * @param width The width of the button.
 * @param height The height of the button.
 * @param callback The callback function to call when the button is clicked.
 * @return A pointer to the newly created GooeyButton object.
 */
GooeyButton *GooeyButton_Create(const char *label, int x, int y,
                                int width, int height, void (*callback)());

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

/**
 * @brief Sets the text of the button.
 *
 * This function updates the label text displayed on the button.
 *
 * @param button The button to set the text for.
 * @param text The new text to display on the button.
 */
void GooeyButton_SetText(GooeyButton *button, const char *text);

/**
 * @brief Draws the button on the window.
 *
 * This function renders the button on the specified window, including its label and any visual states
 * (e.g., highlighted or clicked).
 *
 * @param win The window to draw the button on.
 */
void GooeyButton_Draw(GooeyWindow *win);

/**
 * @brief Highlights or unhighlights a button.
 *
 * This function visually indicates whether a button is highlighted, which can be used to show focus
 * or selection.
 *
 * @param button A pointer to the button to modify.
 * @param is_highlighted `true` to highlight the button; `false` to remove the highlight.
 */
void GooeyButton_SetHighlight(GooeyButton *button, bool is_highlighted);

#endif