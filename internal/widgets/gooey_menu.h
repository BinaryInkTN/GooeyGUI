#ifndef GOOEY_MENU_H
#define GOOEY_MENU_H

#include "gooey_widgets_internal.h"

/**
 * @brief Sets the menu for the Gooey window.
 *
 * Initializes and sets the menu for the given window.
 *
 * @param win The window to set the menu for.
 * @return A new GooeyMenu object.
 */
GooeyMenu *GooeyMenu_Set(GooeyWindow *win);

/**
 * @brief Adds a child menu item to the window's menu.
 *
 * @param win The window to add the menu item to.
 * @param title The title of the menu item.
 * @return A new GooeyMenuChild object.
 */
GooeyMenuChild *GooeyMenu_AddChild(GooeyWindow *win, char *title);

/**
 * @brief Handles menu item click events.
 *
 * @param win The window containing the menu.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 */
void GooeyMenu_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Adds a menu item to a menu child.
 *
 * @param child The menu child to add the item to.
 * @param title The title of the menu item.
 * @param callback The callback function to call when the item is selected.
 */
void GooeyMenuChild_AddElement(GooeyMenuChild *child, char *title, void (*callback)());


void GooeyMenu_Draw(GooeyWindow *win);


#endif