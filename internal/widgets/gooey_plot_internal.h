/**
 * @file gooey_plot.h
 * @brief Gooey Plot API - Provides various types of plots for visualization.
 * 
 * This API allows the addition, drawing, and updating of different types of plots 
 * within a Gooey window. It supports various plot types such as line charts, 
 * bar charts, etc., and enables dynamic data updates for real-time visualizations.
 * 
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_PLOT_INTERNAL_H
#define GOOEY_PLOT_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_PLOT)

/**
 * @brief Draws the plot in the specified Gooey window.
 *
 * This function renders the plot inside the given window. It should be called
 * whenever plot data is added or updated to refresh the visualization.
 *
 * @param win Pointer to the Gooey window where the plot will be drawn.
 */
void GooeyPlot_Draw(GooeyWindow *win);

#endif // ENABLE_PLOT

#endif /* GOOEY_PLOT_INTERNAL_H */
