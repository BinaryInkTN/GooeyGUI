/**
 * @file gooey_label.h
 * @brief Header file for the GooeyLabel module.
 *
 * Provides functions for creating, modifying, and rendering text labels
 * within a GooeyWindow.
 * 
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_LABEL_H
#define GOOEY_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_LABEL)

/**
 * @brief Creates a Label.
 *
 * Creates a new GooeyLabel with the given text, font size, and position.
 *
 * @param text The text to display on the label.
 * @param font_size The font size of the label's text.
 * @param x The x-coordinate of the label's position.
 * @param y The y-coordinate of the label's position.
 * @return A pointer to the newly created GooeyLabel object.
 */
GooeyLabel *GooeyLabel_Create(const char *text, float font_size, int x, int y);

/**
 * @brief Sets the text of an existing label.
 *
 * Updates the text displayed by a given GooeyLabel.
 *
 * @param label The label to update.
 * @param text The new text to display on the label.
 */
void GooeyLabel_SetText(GooeyLabel *label, const char *text);

/**
 * @brief Sets the text color of a label.
 *
 * Changes the text color of a label to the specified color.
 *
 * @param label A pointer to the label whose color is to be changed.
 * @param color An unsigned long representing the color (e.g., 0xFF0000 for red).
 */
void GooeyLabel_SetColor(GooeyLabel *label, unsigned long color);

#endif // ENABLE_LABEL

#ifdef __cplusplus
}
#endif

#endif // GOOEY_LABEL_H
