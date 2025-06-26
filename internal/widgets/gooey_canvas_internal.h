/**
 * @file gooey_canvas_internal.h
 * @brief Internal header for the GooeyCanvas module.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * Provides functions to create and manipulate a user-defined canvas
 * for drawing primitives such as rectangles, lines, and arcs.
 */

#ifndef GOOEY_CANVAS_INTERNAL_H
#define GOOEY_CANVAS_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_CANVAS)

/**
 * @brief Draws the canvas onto the specified window.
 *
 * This function updates the window with the current contents of the canvas.
 *
 * @param window The window onto which the canvas is drawn.
 */
void GooeyCanvas_Draw(GooeyWindow *window);

/**
 * @brief Handles click events on the canvas.
 *
 * Processes a click at the specified coordinates on the canvas,
 * typically to support interaction with canvas elements.
 *
 * @param win The window containing the canvas.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 */
void GooeyCanvas_HandleClick(GooeyWindow *win, int x, int y);

#endif // ENABLE_CANVAS

#ifdef __cplusplus
}
#endif

#endif // GOOEY_CANVAS_INTERNAL_H
