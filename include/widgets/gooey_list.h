/**
 * @file gooey_list.h
 * @brief Header file for the GooeyList widget.
 *
 * Provides functionality for creating and managing list widgets within a GooeyWindow.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

 #ifndef GOOEY_LIST_H
 #define GOOEY_LIST_H
 
 #include "common/gooey_common.h"
 #include "event/gooey_event.h"
 
 /**
  * @brief Adds a list widget to the specified window.
  *
  * Creates a new list widget and attaches it to the given window.
  *
  * @param window The window to which the list widget will be added.
  * @param x The x-coordinate of the list widget's position.
  * @param y The y-coordinate of the list widget's position.
  * @param width The width of the list widget.
  * @param height The height of the list widget.
  * @param callback The function to be called when an item is selected.
  * @return A pointer to the newly created GooeyList object.
  */
 GooeyList *GooeyList_Add(GooeyWindow *window, int x, int y, int width, int height, void (*callback)(int index));
 
 /**
  * @brief Adds an item to the specified list widget.
  *
  * @param list The list widget to which the item will be added.
  * @param title The title of the list item.
  * @param description The description of the list item.
  */
 void GooeyList_AddItem(GooeyList *list, const char *title, const char *description);
 
 /**
  * @brief Clears all items from the specified list widget.
  *
  * Removes all list items from the provided list widget.
  *
  * @param list The list widget to be cleared.
  */
 void GooeyList_ClearItems(GooeyList *list);
 
 /**
  * @brief Toggles the visibility of the separator in a list widget.
  *
  * Enables or disables the visual separator between list items.
  *
  * @param list The list widget.
  * @param state `true` to show the separator, `false` to hide it.
  */
 void GooeyList_ShowSeparator(GooeyList *list, bool state);
 
 /**
  * @brief Handles scroll events for a list widget.
  *
  * Processes mouse scroll events to allow vertical navigation within the list.
  *
  * @param window The window containing the list widget.
  * @param event The scroll event.
  * @return `true` if the event was handled, otherwise `false`.
  */
 bool GooeyList_HandleScroll(GooeyWindow *window, GooeyEvent *event);
 
 /**
  * @brief Handles thumb dragging for scrolling a list.
  *
  * Allows users to drag the scrollbar thumb to navigate through the list.
  *
  * @param window The window containing the list widget.
  * @param scroll_event The scroll event triggered by the user.
  * @return `true` if the event was handled, otherwise `false`.
  */
 bool GooeyList_HandleThumbScroll(GooeyWindow *window, GooeyEvent *scroll_event);
 
 /**
  * @brief Handles click events for a list widget.
  *
  * Detects and processes mouse click events on the list widget.
  *
  * @param window The window containing the list widget.
  * @param mouse_x The x-coordinate of the mouse click.
  * @param mouse_y The y-coordinate of the mouse click.
  * @return `true` if the click event was handled, otherwise `false`.
  */
 bool GooeyList_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y);
 
 /**
  * @brief Draws all attached list widgets onto the specified window.
  *
  * Renders the list widget and its items on the window.
  *
  * @param window The window on which the list widget will be drawn.
  */
 void GooeyList_Draw(GooeyWindow *window);
 
 #endif // GOOEY_LIST_H
 