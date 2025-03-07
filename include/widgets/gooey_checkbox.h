#ifndef GOOEY_CHECKBOX_H
#define GOOEY_CHECKBOX_H

#include "common/gooey_common.h"


/**
 * @brief Adds a checkbox to the window.
 *
 * @param win The window to add the checkbox to.
 * @param x The x-coordinate of the checkbox's position.
 * @param y The y-coordinate of the checkbox's position.
 * @param label The label for the checkbox.
 * @param callback The callback function to call when the checkbox is clicked.
 * @return A new GooeyCheckbox object.
 */
GooeyCheckbox *GooeyCheckbox_Add(GooeyWindow *win, int x, int y, char *label,
                                 void (*callback)(bool checked));

/**
 * @brief Handles checkbox click events.
 *
 * @param win The window containing the checkbox.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return True if the checkbox was clicked, false otherwise.
 */
bool GooeyCheckbox_HandleClick(GooeyWindow *win, int x, int y);


void GooeyCheckbox_Draw(GooeyWindow *win);


#endif