#ifndef GOOEY_SLIDER_H
#define GOOEY_SLIDER_H

#include "common/gooey_common.h"
#include <stdbool.h>

/**
 * @brief Adds a slider to the window.
 *
 * This function creates and adds a slider to the specified window at the given position
 * with a defined width and range. The slider's value changes when dragged, and a callback
 * function is invoked to notify when the slider value changes.
 *
 * @param x The x-coordinate of the slider's position.
 * @param y The y-coordinate of the slider's position.
 * @param width The width of the slider.
 * @param min_value The minimum value of the slider.
 * @param max_value The maximum value of the slider.
 * @param show_hints Whether to show hints for the slider.
 * @param callback The callback function to call when the slider value changes.
 * 
 * @return A new GooeySlider object.
 */
GooeySlider *GooeySlider_Add( int x, int y, int width,
                             long min_value, long max_value, bool show_hints,
                             void (*callback)(long value));

/**
 * @brief Gets the current value of the slider.
 *
 * This function retrieves the current value of the slider. The value is within
 * the range specified by the slider's minimum and maximum values.
 *
 * @param slider The slider to get the value from.
 * 
 * @return The current value of the slider.
 */
long GooeySlider_GetValue(GooeySlider *slider);

/**
 * @brief Sets the value of the slider.
 *
 * This function sets the slider's value to the specified value within the slider's
 * valid range (between the minimum and maximum values).
 *
 * @param slider The slider to set the value for.
 * @param value The new value for the slider.
 */
void GooeySlider_SetValue(GooeySlider *slider, long value);


#endif /* GOOEY_SLIDER_H */
