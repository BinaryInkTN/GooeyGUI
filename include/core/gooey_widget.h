#ifndef GOOEY_WIDGET_H
#define GOOEY_WIDGET_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the visibility state of a widget.
 *
 * @param widget Pointer to the widget.
 * @param state True to make visible, false to hide.
 */
void GooeyWidget_MakeVisible(void* widget, bool state);

/**
 * @brief Moves the widget to a new position.
 *
 * @param widget Pointer to the widget.
 * @param x New x-coordinate.
 * @param y New y-coordinate.
 */
void GooeyWidget_MoveTo(void* widget, int x, int y);

/**
 * @brief Resizes the widget.
 *
 * @param widget Pointer to the widget.
 * @param w New width.
 * @param h New height.
 */
void GooeyWidget_Resize(void* widget, int w, int h);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_WIDGET_H */
