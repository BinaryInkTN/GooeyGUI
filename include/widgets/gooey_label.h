#ifndef GOOEY_LABEL_H
#define GOOEY_LABEL_H

#include "common/gooey_common.h"

/**
 * @brief Adds a label to the window.
 *
 * @param win The window to add the label to.
 * @param text The text to display on the label.
 * @param x The x-coordinate of the label's position.
 * @param y The y-coordinate of the label's position.
 * @return A new GooeyLabel object.
 */
GooeyLabel *GooeyLabel_Add(GooeyWindow *win, const char *text, float font_size, int x, int y);

/**
 * @brief Sets the text of the label.
 *
 * @param label The label to set the text for.
 * @param text The new text to display on the label.
 */
void GooeyLabel_SetText(GooeyLabel *label, const char *text);

/**
 * @brief Draws the label on the window.
 *
 * @param win The window to draw the label on.
 */
void GooeyLabel_Draw(GooeyWindow *win);

/**
 * @brief Sets the color of a label.
 *
 * Changes the text color of a label to the specified color string.
 *
 * @param label A pointer to the label whose color is to be set.
 * @param color A string representing the color (e.g., "red", "#FF0000").
 */
void GooeyLabel_SetColor(GooeyLabel *label, const char *color);


#endif