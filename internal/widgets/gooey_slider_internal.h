#ifndef GOOEY_SLIDER_INTERNAL_H
#define GOOEY_SLIDER_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_SLIDER)
#include <stdbool.h>

/**
 * @brief Handles slider drag events.
 *
 * Processes slider drag events by checking if the event corresponds to dragging
 * a slider and updates the slider's value accordingly.
 *
 * @param win The window containing the slider.
 * @param event The current event.
 * @return True if the slider was dragged, false otherwise.
 */
bool GooeySlider_HandleDrag(GooeyWindow *win, void *event);

/**
 * @brief Draws the slider on the window.
 *
 * Renders the slider on the specified window. Should be called after updating
 * the slider's state to visually reflect changes.
 *
 * @param win The window to draw the slider on.
 */
void GooeySlider_Draw(GooeyWindow *win);

#endif // ENABLE_SLIDER

#endif /* GOOEY_SLIDER_INTERNAL_H */
