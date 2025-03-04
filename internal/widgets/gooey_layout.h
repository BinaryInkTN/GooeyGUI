#ifndef GOOEY_LAYOUT_H
#define GOOEY_LAYOUT_H

#include "gooey_widgets_internal.h"

/**
 * @brief Creates a layout for the specified window.
 *
 * Initializes a new layout with a specified layout type, position, and size
 * within the window.
 *
 * @param win The window to attach the layout to.
 * @param layout_type The type of layout (e.g., grid, box).
 * @param x The x-coordinate of the layout's position.
 * @param y The y-coordinate of the layout's position.
 * @param width The width of the layout.
 * @param height The height of the layout.
 * @return A new GooeyLayout object.
 */
GooeyLayout *GooeyLayout_Create(GooeyWindow *win, GooeyLayoutType layout_type,
                                int x, int y, int width, int height);

/**
 * @brief Adds a child widget to a layout.
 *
 * @param layout The layout to which the widget will be added.
 * @param widget The widget to add to the layout.
 */
void GooeyLayout_AddChild(GooeyLayout *layout, void *widget);

/**
 * @brief Builds the layout, arranging all child widgets according to the layout type.
 *
 * @param layout The layout to build.
 */
void GooeyLayout_Build(GooeyLayout *layout);


#endif