#ifndef GOOEY_SLIDER_H
#define GOOEY_SLIDER_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_SLIDER)
#include <stdbool.h>

/**
 * @brief Creates and adds a slider widget to the window.
 *
 * Adds a slider at the specified (x, y) position with a given width and value range.
 * The slider invokes the callback whenever its value changes.
 *
 * @param x The x-coordinate of the slider.
 * @param y The y-coordinate of the slider.
 * @param width The width of the slider.
 * @param min_value The minimum value allowed for the slider.
 * @param max_value The maximum value allowed for the slider.
 * @param show_hints Whether to display value hints alongside the slider.
 * @param callback Function called when the slider value changes, receiving the new value.
 * 
 * @return Pointer to the created GooeySlider.
 */
GooeySlider *GooeySlider_Create(int x, int y, int width,
                                long min_value, long max_value, bool show_hints,
                                void (*callback)(long value));

/**
 * @brief Retrieves the current value of the slider.
 *
 * Returns the current slider value, guaranteed to be within the configured range.
 *
 * @param slider The slider instance.
 * 
 * @return The current value of the slider.
 */
long GooeySlider_GetValue(GooeySlider *slider);

/**
 * @brief Sets the slider's value.
 *
 * Updates the slider's position and value to the specified value within its range.
 *
 * @param slider The slider instance.
 * @param value The new value to set.
 */
void GooeySlider_SetValue(GooeySlider *slider, long value);

#endif // ENABLE_SLIDER

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_SLIDER_H */
