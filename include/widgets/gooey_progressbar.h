#ifndef GOOEY_PROGRESSBAR_H
#define GOOEY_PROGRESSBAR_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_PROGRESSBAR)

/**
 * @brief Creates a progress bar widget.
 *
 * Initializes a new progress bar at the specified position and size,
 * with an initial value.
 *
 * @param x The x-coordinate of the progress bar.
 * @param y The y-coordinate of the progress bar.
 * @param width The width of the progress bar.
 * @param height The height of the progress bar.
 * @param initial_value The initial progress value.
 * @return Pointer to the newly created GooeyProgressBar.
 */
GooeyProgressBar *GooeyProgressBar_Create(int x, int y, int width, int height, long initial_value);

/**
 * @brief Updates the value of an existing progress bar.
 *
 * Changes the displayed progress to the new value.
 *
 * @param progressbar Pointer to the progress bar to update.
 * @param new_value The new progress value.
 */
void GooeyProgressBar_Update(GooeyProgressBar *progressbar, long new_value);

#endif // ENABLE_PROGRESSBAR

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GOOEY_PROGRESSBAR_H
