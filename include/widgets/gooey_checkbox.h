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

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_CHECKBOX)

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

#endif // ENABLE_CHECKBOX

#ifdef __cplusplus
}
#endif

#endif // GOOEY_CHECKBOX_H
