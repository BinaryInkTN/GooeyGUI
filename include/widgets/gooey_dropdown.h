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

 #ifndef GOOEY_DROPDOWN_H
 #define GOOEY_DROPDOWN_H
 
 #include "common/gooey_common.h"
 
 /**
  * @brief Adds a dropdown menu to the specified window.
  *
  * This function creates a new dropdown menu with the given list of options 
  * and attaches it to the specified GooeyWindow.
  *
  * @param win The window to add the dropdown menu to.
  * @param x The x-coordinate of the dropdown's position.
  * @param y The y-coordinate of the dropdown's position.
  * @param width The width of the dropdown menu.
  * @param height The height of the dropdown menu.
  * @param options The array of strings representing the dropdown menu options.
  * @param num_options The total number of options in the dropdown.
  * @param callback The function to call when an option is selected. 
  *                 It receives the index of the selected option.
  * @return A pointer to the newly created GooeyDropdown object.
  */
 GooeyDropdown *GooeyDropdown_Add(GooeyWindow *win, int x, int y, int width,
                                  int height, const char **options,
                                  int num_options,
                                  void (*callback)(int selected_index));
 
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
 
 #endif // GOOEY_DROPDOWN_H
 