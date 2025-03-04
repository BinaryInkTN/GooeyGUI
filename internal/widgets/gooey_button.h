#ifndef GOOEY_BUTTON_H
#define GOOEY_BUTTON_H

#include "gooey_widgets_internal.h"

#include <stdbool.h>

/**
 * @brief Adds a button to the window.
 *
 * @param win The window to add the button to.
 * @param label The label to display on the button.
 * @param x The x-coordinate of the button's position.
 * @param y The y-coordinate of the button's position.
 * @param width The width of the button.
 * @param height The height of the button.
 * @param callback The callback function to call when the button is clicked.
 * @return A new GooeyButton object.
 */
GooeyButton *GooeyButton_Add(GooeyWindow *win, const char *label, int x, int y,
                             int width, int height, void (*callback)());

/**
 * @brief Handles button click events.
 *
 * @param win The window containing the button.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return True if the button was clicked, false otherwise.
 */
bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Sets the text of the button.
 *
 * @param button The button to set the text for.
 * @param text The new text to display on the button.
 */
void GooeyButton_SetText(GooeyButton *button, const char *text);

/**
 * @brief Draws the button on the window.
 *
 * @param win The window to draw the button on.
 */
void GooeyButton_Draw(GooeyWindow *win);

/**
 * @brief Highlights or unhighlights a button.
 *
 * This function visually indicates whether a button is highlighted.
 *
 * @param button A pointer to the button to modify.
 * @param is_highlighted `true` to highlight the button; `false` to remove the highlight.
 */
void GooeyButton_SetHighlight(GooeyButton *button, bool is_highlighted);


#endif