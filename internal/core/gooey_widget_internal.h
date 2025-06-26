#ifndef GOOEY_WIDGET_INTERNAL_H
#define GOOEY_WIDGET_INTERNAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the visibility of the widget.
 *
 * @param widget Pointer to the widget.
 * @param state true to make visible, false to hide.
 */
void GooeyWidget_MakeVisible_Internal(void *widget, bool state);

/**
 * @brief Moves the widget to a new position.
 *
 * @param widget Pointer to the widget.
 * @param x New x-coordinate.
 * @param y New y-coordinate.
 */
void GooeyWidget_MoveTo_Internal(void *widget, int x, int y);

/**
 * @brief Resizes the widget to the given dimensions.
 *
 * @param widget Pointer to the widget.
 * @param w New width.
 * @param h New height.
 */
void GooeyWidget_Resize_Internal(void *widget, int w, int h);

#ifdef __cplusplus
}
#endif

#endif /* GOOEY_WIDGET_INTERNAL_H */
