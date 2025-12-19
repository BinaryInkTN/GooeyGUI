#ifndef GOOEY_METER_H
#define GOOEY_METER_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_METER)

/**
 * @brief Creates a meter widget.
 *
 * Initializes a new GooeyMeter widget at the specified position and size,
 * with an initial value, a label, and an optional icon.
 *
 * @param x The x-coordinate of the meter's position.
 * @param y The y-coordinate of the meter's position.
 * @param width The width of the meter.
 * @param height The height of the meter.
 * @param initial_value The initial value displayed by the meter.
 * @param label A text label to display with the meter.
 * @param icon_path Path to an icon image to display with the meter (can be NULL).
 * @return A pointer to the newly created GooeyMeter object.
 */
GooeyMeter *GooeyMeter_Create(int x, int y, int width, int height, long initial_value, const char *label, const char *icon_path);

/**
 * @brief Updates the value displayed by the meter.
 *
 * @param meter The GooeyMeter instance to update.
 * @param new_value The new value to set.
 */
void GooeyMeter_Update(GooeyMeter *meter, long new_value);

#endif // ENABLE_METER

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GOOEY_METER_H
