/**
 * @file gooey_menu.h
 * @brief Header file for the Gooey menu system.
 *
 * Provides functions to create and manage menus and menu items within a Gooey window.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_MENU_H
#define GOOEY_MENU_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_MENU)

/**
 * @brief Sets the menu for the specified Gooey window.
 *
 * Initializes and attaches a menu to the given window.
 *
 * @param win The window to set the menu for.
 * @return A pointer to the newly created GooeyMenu object.
 */
GooeyMenu *GooeyMenu_Set(GooeyWindow *win);

/**
 * @brief Adds a child menu item to the window's menu.
 *
 * Creates a submenu or a category inside the main menu.
 *
 * @param win The window to which the menu child will be added.
 * @param title The title of the menu child.
 * @return A pointer to the newly created GooeyMenuChild object.
 */
GooeyMenuChild *GooeyMenu_AddChild(GooeyWindow *win, char *title);

/**
 * @brief Adds an element (option) to a menu child.
 *
 * @param child The menu child to which the item will be added.
 * @param title The title of the menu item.
 * @param callback The function to be called when the menu item is selected.
 */
void GooeyMenuChild_AddElement(GooeyMenuChild *child, char *title, void (*callback)(void));

#endif // ENABLE_MENU

#ifdef __cplusplus
}
#endif

#endif // GOOEY_MENU_H
