#ifndef GOOEY_LIST_INTERNAL_H
#define GOOEY_LIST_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_LIST)

/**
 * @brief Handles scroll events for a list widget.
 *
 * Processes mouse scroll events to allow vertical navigation within the list.
 *
 * @param window The window containing the list widget.
 * @param event The scroll event.
 * @return true if the event was handled, otherwise false.
 */
bool GooeyList_HandleScroll(GooeyWindow *window, void *event);

/**
 * @brief Handles thumb dragging for scrolling a list.
 *
 * Allows users to drag the scrollbar thumb to navigate through the list.
 *
 * @param window The window containing the list widget.
 * @param scroll_event The scroll event triggered by the user.
 * @return true if the event was handled, otherwise false.
 */
bool GooeyList_HandleThumbScroll(GooeyWindow *window, void *scroll_event);

/**
 * @brief Handles click events for a list widget.
 *
 * Detects and processes mouse click events on the list widget.
 *
 * @param window The window containing the list widget.
 * @param mouse_x The x-coordinate of the mouse click.
 * @param mouse_y The y-coordinate of the mouse click.
 * @return true if the click event was handled, otherwise false.
 */
bool GooeyList_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y);

/**
 * @brief Handles hover events for a list widget.
 *
 * Detects and processes mouse hover events over the list widget.
 *
 * @param window The window containing the list widget.
 * @param mouse_x The x-coordinate of the mouse pointer.
 * @param mouse_y The y-coordinate of the mouse pointer.
 * @return true if the hover event was handled, otherwise false.
 */
bool GooeyList_HandleHover(GooeyWindow *window, int mouse_x, int mouse_y);

/**
 * @brief Draws all attached list widgets onto the specified window.
 *
 * Renders the list widget and its items on the window.
 *
 * @param window The window on which the list widget will be drawn.
 */
void GooeyList_Draw(GooeyWindow *window);

#endif // ENABLE_LIST

#endif // GOOEY_LIST_INTERNAL_H
