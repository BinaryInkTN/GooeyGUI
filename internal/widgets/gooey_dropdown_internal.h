/**
 * @file gooey_dropdown.h
 * @brief Header file for the GooeyDropdown module.
 *
 * Provides functions to create, handle, and render dropdown menus 
 * within a GooeyWindow.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

 #ifndef GOOEY_DROPDOWN_INTERNAL_H
 #define GOOEY_DROPDOWN_INTERNAL_H
 
 #include "common/gooey_common.h"
 
 /**
  * @brief Handles click events for dropdown menus within the specified window.
  *
  * This function detects whether a dropdown menu has been clicked and 
  * updates its state accordingly.
  *
  * @param win The window containing the dropdown menu.
  * @param x The x-coordinate of the click event.
  * @param y The y-coordinate of the click event.
  * @return True if the dropdown menu was clicked, false otherwise.
  */
 bool GooeyDropdown_HandleClick(GooeyWindow *win, int x, int y);
 
 /**
  * @brief Draws all dropdown menus within the specified window.
  *
  * This function renders all GooeyDropdown elements that belong to 
  * the given window, ensuring they are displayed with the correct 
  * selection state.
  *
  * @param win The window containing the dropdown menu.
  */
 void GooeyDropdown_Draw(GooeyWindow *win);
 
 #endif // GOOEY_DROPDOWN_INTERNAL_H
 