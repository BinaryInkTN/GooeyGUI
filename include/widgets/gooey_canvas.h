#ifndef GOOEY_CANVAS_H
#define GOOEY_CANVAS_H

#include "common/gooey_common.h"


/**
 * @brief Adds a canvas to the window.
 *
 * @param win The window to add the canvas to.
 * @param x The x-coordinate of the canvas' position.
 * @param y The y-coordinate of the canvas' position.
 * @param width The width of the canvas.
 * @param height The height of the canvas.
 * @return A new GooeyCanvas object.
 */
GooeyCanvas *GooeyCanvas_Add(GooeyWindow *win, int x, int y, int width,
                             int height);

/**
 * @brief Draws a rectangle onto the user-defined canvas.
 *
 * @param canvas The user-defined canvas.
 * @param x The x-coordinate of the rectangle.
 * @param y The y-coordinate of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color_hex The color of the rectangle in hexadecimal.
 * @param is_filled If true, the rectangle is filled with a solid color. If false it is hollow.
 */
void GooeyCanvas_DrawRectangle(GooeyCanvas *canvas, int x, int y, int width, int height, unsigned long color_hex, bool is_filled);

/**
 * @brief Draws a line onto the user-defined canvas.
 *
 * @param canvas The user-defined canvas.
 * @param x1 The x-coordinate of the origin point of the line.
 * @param y1 The y-coordinate of the origin point of the line.
 * @param x2 The x-coordinate of the last point of the line.
 * @param y2 The y-coordinate of the last point of the line.
 * @param color_hex The color of the line in hexadecimal.
 */
void GooeyCanvas_DrawLine(GooeyCanvas *canvas, int x1, int y1, int x2, int y2, unsigned long color_hex);

/**
 * @brief Draws an arc onto the user-defined canvas.
 *
 * @param canvas The user-defined canvas.
 * @param x_center The x-coordinate of the arc.
 * @param y_center The y-coordinate of the arc.
 * @param width The width of the arc.
 * @param height The height of the arc.
 * @param angle1 starting angle of the arc.
 * @param angle2 end angle of the arc.
 */
void GooeyCanvas_DrawArc(GooeyCanvas *canvas, int x_center, int y_center, int width, int height, int angle1, int angle2);

/**
 * @brief Sets foreground of the user-defined canvas.
 *
 * @param canvas The user-defined canvas.
 * @param color_hex The foreground color in hexadecimal.
 */
void GooeyCanvas_SetForeground(GooeyCanvas *canvas, unsigned long color_hex);

void GooeyCanvas_Draw(GooeyWindow* window);


#endif