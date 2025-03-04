#ifndef GOOEY_DROPDOWN_H
#define GOOEY_DROPDOWN_H

#include "gooey_widgets_internal.h"

/**
 * @brief Adds a dropdown menu to the window.
 *
 * @param win The window to add the dropdown menu to.
 * @param x The x-coordinate of the dropdown's position.
 * @param y The y-coordinate of the dropdown's position.
 * @param width The width of the dropdown menu.
 * @param height The height of the dropdown menu.
 * @param options The list of options for the dropdown menu.
 * @param num_options The number of options.
 * @param callback The callback function to call when an option is selected.
 * @return A new GooeyDropdown object.
 */
GooeyDropdown *GooeyDropdown_Add(GooeyWindow *win, int x, int y, int width,
                                 int height, const char **options,
                                 int num_options,
                                 void (*callback)(int selected_index));

/**
 * @brief Handles dropdown menu click events.
 *
 * @param win The wi>ndow containing the dropdown menu.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return True if the dropdown menu was clicked, false otherwise.
 */
bool GooeyDropdown_HandleClick(GooeyWindow *win, int x, int y);
/**
 * @brief Draws the drop down menu on the window.
 *
 * @param win The window to draw the drop down menu.
 */
void GooeyDropdown_Draw(GooeyWindow *win);



#endif