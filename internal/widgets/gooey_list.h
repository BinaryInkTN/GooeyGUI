#ifndef GOOEY_LIST_H
#define GOOEY_LIST_H

#include "gooey_widgets_internal.h"
#include "gooey_event_internal.h"

/**
 * @brief Adds a list widget to the window specified.
 *
 * @param window The window which you're adding the list widget to.
 * @param x The x-coordinate of the list widget.
 * @param y The y-coordinate of the list widget.
 * @param width The width of the list widget.
 * @param height The height of the list widget.
 * @param callback The callback attached to the said item.
 */
GooeyList *GooeyList_Add(GooeyWindow *window, int x, int y, int width, int height, void (*callback)(int index));

/**
 * @brief Adds a list widget to the window specified.
 *
 * @param list The List widget you're adding the item to.
 * @param title The title of the list widget item.
 * @param description The description of the list widget item.
 */
void GooeyList_AddItem(GooeyList *list, const char *title, const char *description);

/**
 * @brief Clears all items from a list widget.
 *
 * @param list The List widget from which you're clearing all items.
 */
void GooeyList_ClearItems(GooeyList *list);

/**
 * @brief Shows or hides separator in a list widget
 *
 * @param list The List widget.
 * @param state show (true) / hide (false)
 */
void GooeyList_ShowSeparator(GooeyList *list, bool state);


/**
 * @brief Handles scroll events.
 *
 * @param window The window containing the list.
 * @param event The scroll event.
 * @return if mouse scroll event then returns true, otherwise returns false.
 */
bool GooeyList_HandleScroll(GooeyWindow *window, GooeyEvent *event);

/**
 * Handles thumb dragging for a GooeyList.
 * @param window Pointer to the GooeyWindow.
 * @param scroll_event Pointer to the GooeyEvent.
 * @return True if the event was handled, false otherwise.
 */
bool GooeyList_HandleThumbScroll(GooeyWindow *window, GooeyEvent *scroll_event);

bool GooeyList_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y);

/**
 * @brief Draws All attached list widgets onto window.
 *
 * @param window The window which you're adding the list widget to.
 */
void GooeyList_Draw(GooeyWindow *window);

#endif