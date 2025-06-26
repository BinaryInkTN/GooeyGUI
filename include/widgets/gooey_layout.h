/**
 * @file gooey_layout.h
 * @brief Header file for the GooeyLayout module.
 *
 * Provides functionality for managing and arranging widgets within a structured layout
 * inside a GooeyWindow.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_LAYOUT_H
#define GOOEY_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_LAYOUT)

/**
 * @brief Creates a layout.
 *
 * Initializes a new layout with a specified type, position, and size.
 *
 * @param layout_type The type of layout (e.g., grid, box).
 * @param x The x-coordinate of the layout's position.
 * @param y The y-coordinate of the layout's position.
 * @param width The width of the layout.
 * @param height The height of the layout.
 * @return A pointer to the newly created GooeyLayout object.
 */
GooeyLayout *GooeyLayout_Create(GooeyLayoutType layout_type,
                                int x, int y, int width, int height);

/**
 * @brief Adds a child widget to the specified layout.
 *
 * Registers a widget as a child of the given layout, allowing it to be positioned
 * according to the layout's rules.
 *
 * @param window The GooeyWindow containing the layout.
 * @param layout The layout to which the widget will be added.
 * @param widget A pointer to the widget to add.
 */
void GooeyLayout_AddChild(GooeyWindow* window, GooeyLayout *layout, void *widget);

/**
 * @brief Finalizes and builds the layout.
 *
 * This function applies the layout rules and arranges all child widgets accordingly.
 *
 * @param layout The layout to build.
 */
void GooeyLayout_Build(GooeyLayout* layout);

#endif // ENABLE_LAYOUT

#ifdef __cplusplus
}
#endif

#endif // GOOEY_LAYOUT_H
