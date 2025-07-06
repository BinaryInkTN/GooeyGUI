#ifndef GOOEY_SWITCH_H
#define GOOEY_SWITCH_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_SWITCH)
#include <stdbool.h>

/**
 * @brief Creates and adds a slider widget to the window.
 *
 * Adds a slider at the specified (x, y) position with a given width and value range.
 * The slider invokes the callback whenever its value changes.
 *
 * @param x The x-coordinate of the slider.
 * @param y The y-coordinate of the slider.
 * @param bool init state of the switch.

 * @param show_hints Whether to display value hints alongside the slider.
 * @param callback Function called when the slider value changes, receiving the new value.
 * 
 * @return Pointer to the created GooeySlider.
 */
GooeySwitch *GooeySwitch_Create(int x, int y, bool IsToggled, bool show_hints,
                                void (*callback)(bool value));

/**
 * @brief Retrieves the current value of the slider.
 *
 * Returns the current slider value, guaranteed to be within the configured range.
 *
 * @param slider The slider instance.
 * 
 * @return The current value of the slider.
 */
bool GooeySwitch_GetState(GooeySwitch *gswitch);

/**
 * @brief Sets the slider's value.
 *
 * Updates the slider's position and value to the specified value within its range.
 *
 * @param slider The slider instance.

 */
void GooeySwitch_Toggle(GooeySwitch *gswitch);

#endif // ENABLE_SLIDER

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_SLIDER_H */
