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

 #ifndef GOOEY_PLOT_H
 #define GOOEY_PLOT_H
 
 #include "common/gooey_common.h"
 
 /**
  * @brief Creates a plot widget.
  *
  * This function allows adding various types of plots to a given Gooey window.
  * The plot type is specified by the user, and the plot data is provided through
  * a GooeyPlotData structure.
  *
  * @param plot_type The type of plot to be added (e.g., LINE, BAR).
  * @param data Pointer to the structure containing the plot data.
  * @param x The x-coordinate of the plot's position.
  * @param y The y-coordinate of the plot's position.
  * @param width The width of the plot.
  * @param height The height of the plot.
  * 
  * @return A pointer to the newly created GooeyPlot.
  */
 GooeyPlot *GooeyPlot_Create(GOOEY_PLOT_TYPE plot_type, GooeyPlotData *data, int x, int y, int width, int height); 
 
 /**
  * @brief Draws the plot in the specified Gooey window.
  *
  * This function is used to render the plot within the specified window.
  * It should be called after the plot data has been added or updated to
  * visualize the plot on the screen.
  *
  * @param win Pointer to the Gooey window where the plot will be drawn.
  */
 void GooeyPlot_Draw(GooeyWindow* win);
 
 /**
  * @brief Updates the data of an existing plot.
  *
  * This function updates the plot with new data. It is intended to modify the
  * content of an existing plot, maintaining its type and other configurations.
  *
  * @param plot Pointer to the GooeyPlot to be updated.
  * @param new_data Pointer to the new plot data.
  */
 void GooeyPlot_Update(GooeyPlot *plot, GooeyPlotData *new_data);
 
 #endif /* GOOEY_PLOT_H */
 