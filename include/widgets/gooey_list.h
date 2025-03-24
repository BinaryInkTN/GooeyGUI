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
 
 /**
  * @brief Creates a widget.
  *
  * Creates a new list widget and attaches it to the given window.
  *
  * @param x The x-coordinate of the list widget's position.
  * @param y The y-coordinate of the list widget's position.
  * @param width The width of the list widget.
  * @param height The height of the list widget.
  * @param callback The function to be called when an item is selected.
  * @return A pointer to the newly created GooeyList object.
  */
 GooeyList *GooeyList_Create(int x, int y, int width, int height, void (*callback)(int index));
 
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
 
 #endif // GOOEY_LIST_H
 