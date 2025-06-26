#ifndef GOOEY_PLOT_H
#define GOOEY_PLOT_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_PLOT)

/**
 * @brief Creates a plot widget.
 *
 * Adds a plot widget of the specified type to the given position and size,
 * using the provided plot data.
 *
 * @param plot_type The type of plot to be created (e.g., LINE, BAR).
 * @param data Pointer to the plot data structure.
 * @param x The x-coordinate of the plot's position.
 * @param y The y-coordinate of the plot's position.
 * @param width The width of the plot widget.
 * @param height The height of the plot widget.
 * @return Pointer to the newly created GooeyPlot object.
 */
GooeyPlot *GooeyPlot_Create(GOOEY_PLOT_TYPE plot_type, GooeyPlotData *data, int x, int y, int width, int height);

/**
 * @brief Updates an existing plot with new data.
 *
 * Updates the content of the given plot widget while maintaining
 * its configuration and type.
 *
 * @param plot Pointer to the plot widget to update.
 * @param new_data Pointer to the new data to update the plot with.
 */
void GooeyPlot_Update(GooeyPlot *plot, GooeyPlotData *new_data);

#endif // ENABLE_PLOT

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GOOEY_PLOT_H
