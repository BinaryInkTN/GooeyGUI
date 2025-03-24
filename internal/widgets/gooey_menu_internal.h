/**
 * @file gooey_menu.h
 * @brief Header file for the Gooey menu system.
 *
 * Provides functions to create and manage menus and menu items within a Gooey window.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_MENU_INTERNAL_H
#define GOOEY_MENU_INTERNAL_H

#include "common/gooey_common.h"

/**
 * @brief Handles menu item click events.
 *
 * Processes user interactions with menu items.
 *
 * @param win The window containing the menu.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return `true` if the menu item was clicked, otherwise `false`.
 */
bool GooeyMenu_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Draws the menu and its items on the window.
 *
 * Renders the menu visually within the specified window.
 *
 * @param win The window where the menu will be drawn.
 */
void GooeyMenu_Draw(GooeyWindow *win);

#endif // GOOEY_MENU_H
