/**
 * @file gooey_checkbox.h
 * @brief Header file for the GooeyCheckbox module.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 * 
 * Provides functions to create, handle, and draw checkboxes within a GooeyWindow.
 */

#ifndef GOOEY_CHECKBOX_H
#define GOOEY_CHECKBOX_H

#include "common/gooey_common.h"

/**
 * @brief Adds a checkbox to the specified window.
 *
 * This function creates a new GooeyCheckbox with the given label.
 *
 * @param x The x-coordinate of the checkbox's position.
 * @param y The y-coordinate of the checkbox's position.
 * @param label The label text displayed next to the checkbox.
 * @param callback The function to call when the checkbox is clicked.
 *                The callback receives a boolean indicating whether
 *                the checkbox is checked (`true`) or unchecked (`false`).
 * @return A pointer to the newly created GooeyCheckbox.
 */
GooeyCheckbox *GooeyCheckbox_Create(int x, int y, char *label,
                                 void (*callback)(bool checked));

/**
 * @brief Handles click events for checkboxes within the specified window.
 *
 * This function checks whether a click event occurred on a checkbox
 * and updates its state accordingly.
 *
 * @param win The window containing the checkbox.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return True if a checkbox was clicked and its state changed, false otherwise.
 */
bool GooeyCheckbox_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Draws all checkboxes within the specified window.
 *
 * This function renders all checkboxes that belong to the given window,
 * ensuring they are displayed with their correct state (checked/unchecked).
 *
 * @param win The window containing the checkboxes.
 */
void GooeyCheckbox_Draw(GooeyWindow *win);

#endif // GOOEY_CHECKBOX_H
