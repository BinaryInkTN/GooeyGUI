
/**
 * @file gooey_canvas.h
 * @brief Header file for the GooeyCanvas module.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * Provides functions to create and manipulate a user-defined canvas
 * for drawing primitives such as rectangles, lines, and arcs.
 */

#ifndef GOOEY_CANVAS_INTERNAL_H
#define GOOEY_CANVAS_INTERNAL_H

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
void GooeyCanvas_HandleClick(GooeyWindow* win, int x, int y);
#endif // GOOEY_CANVAS_INTERNAL_H
#endif