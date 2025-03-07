#ifndef GOOEY_SLIDER_H
#define GOOEY_SLIDER_H


#include "event/gooey_event.h"
#include "common/gooey_common.h"
#include <stdbool.h>

/**
 * @brief Adds a slider to the window.
 *
 * @param win The window to add the slider to.
 * @param x The x-coordinate of the slider's position.
 * @param y The y-coordinate of the slider's position.
 * @param width The width of the slider.
 * @param min_value The minimum value of the slider.
 * @param max_value The maximum value of the slider.
 * @param show_hints Whether to show hints for the slider.
 * @param callback The callback function to call when the slider value changes.
 * @return A new GooeySlider object.
 */
GooeySlider *GooeySlider_Add(GooeyWindow *win, int x, int y, int width,
                             long min_value, long max_value, bool show_hints,
                             void (*callback)(long value));

/**
 * @brief Handles slider drag events.
 *
 * @param win The window containing the slider.
 * @param event The current event.
 * @return True if the slider was dragged, false otherwise.
 */
bool GooeySlider_HandleDrag(GooeyWindow *win, GooeyEvent *event);
/**
 * @brief Gets the current value of the slider.
 *
 * @param slider The slider to get the value from.
 * @return The current value of the slider.
 */
long GooeySlider_GetValue(GooeySlider *slider);

/**
 * @brief Sets the value of the slider.
 *
 * @param slider The slider to set the value for.
 * @param value The new value for the slider.
 */
void GooeySlider_SetValue(GooeySlider *slider, long value);

void GooeySlider_Draw(GooeyWindow *window);

#endif