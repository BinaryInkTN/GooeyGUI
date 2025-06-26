/**
 * @file gooey_canvas.h
 * @brief Header file for the GooeyCanvas module.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * Provides functions to create and manipulate a user-defined canvas
 * for drawing primitives such as rectangles, lines, and arcs.
 */

#ifndef GOOEY_CANVAS_H
#define GOOEY_CANVAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_CANVAS)

/**
 * @brief Adds a canvas to the specified window.
 *
 * This function creates a new GooeyCanvas and attaches it to the given
 * GooeyWindow at the specified coordinates and dimensions.
 *
 * @param x The x-coordinate of the canvas position.
 * @param y The y-coordinate of the canvas position.
 * @param width The width of the canvas.
 * @param height The height of the canvas.
 * @param callback Click callback function.
 * @return A pointer to the newly created GooeyCanvas.
 */
GooeyCanvas *GooeyCanvas_Create(int x, int y, int width,
                                int height, void (*callback)(int x, int y));

/**
 * @brief Draws a rectangle onto the user-defined canvas.
 *
 * The rectangle can be either filled with a solid color or outlined.
 *
 * @param canvas The user-defined canvas.
 * @param x The x-coordinate of the rectangle's top-left corner.
 * @param y The y-coordinate of the rectangle's top-left corner.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color_hex The color of the rectangle in hexadecimal format.
 * @param is_filled If true, the rectangle is filled with a solid color; otherwise, it is hollow.
 * @param thickness The line thickness for the outline.
 * @param is_rounded If true, corners are rounded.
 * @param corner_radius Radius of the rounded corners.
 */
void GooeyCanvas_DrawRectangle(GooeyCanvas *canvas, int x, int y, int width, int height,
                               unsigned long color_hex, bool is_filled, float thickness,
                               bool is_rounded, float corner_radius);

/**
 * @brief Draws a line onto the user-defined canvas.
 *
 * @param canvas The user-defined canvas.
 * @param x1 The x-coordinate of the starting point of the line.
 * @param y1 The y-coordinate of the starting point of the line.
 * @param x2 The x-coordinate of the ending point of the line.
 * @param y2 The y-coordinate of the ending point of the line.
 * @param color_hex The color of the line in hexadecimal format.
 */
void GooeyCanvas_DrawLine(GooeyCanvas *canvas, int x1, int y1, int x2, int y2, unsigned long color_hex);

/**
 * @brief Draws an arc onto the user-defined canvas.
 *
 * The arc is drawn within the specified bounding box and between the given angles.
 *
 * @param canvas The user-defined canvas.
 * @param x_center The x-coordinate of the arc's center.
 * @param y_center The y-coordinate of the arc's center.
 * @param width The width of the arc's bounding box.
 * @param height The height of the arc's bounding box.
 * @param angle1 The starting angle of the arc in degrees.
 * @param angle2 The ending angle of the arc in degrees.
 */
void GooeyCanvas_DrawArc(GooeyCanvas *canvas, int x_center, int y_center, int width,
                         int height, int angle1, int angle2);

/**
 * @brief Sets the foreground color of the user-defined canvas.
 *
 * The foreground color is used for subsequent drawing operations.
 *
 * @param canvas The user-defined canvas.
 * @param color_hex The foreground color in hexadecimal format.
 */
void GooeyCanvas_SetForeground(GooeyCanvas *canvas, unsigned long color_hex);

#endif // ENABLE_CANVAS

#ifdef __cplusplus
}
#endif

#endif // GOOEY_CANVAS_H
