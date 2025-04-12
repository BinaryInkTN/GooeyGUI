#ifndef GOOEY_SLIDER_INTERNAL_H
#define GOOEY_SLIDER_INTERNAL_H

#include "common/gooey_common.h"
#include <stdbool.h>

/**
 * @brief Handles slider drag events.
 *
 * This function is responsible for processing slider drag events. It checks whether
 * the event corresponds to dragging a slider and updates the slider's value accordingly.
 *
 * @param win The window containing the slider.
 * @param event The current event.
 * 
 * @return True if the slider was dragged, false otherwise.
 */
bool GooeySlider_HandleDrag(GooeyWindow *win, void *event);

/**
 * @brief Draws the slider on the window.
 *
 * This function renders the slider on the specified window. It should be called after
 * the slider's state has been updated to reflect the changes visually.
 *
 * @param window The window to draw the slider on.
 */
void GooeySlider_Draw(GooeyWindow *window);

#endif /* GOOEY_SLIDER_H */
