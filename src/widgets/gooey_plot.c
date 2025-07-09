/*
 Copyright (c) 2025 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <widgets/gooey_plot.h>
#if(ENABLE_PLOT)
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"


typedef struct
{
    float x;
    float y;
} DataPoint;

static int compare_data_points(const void *a, const void *b)
{
    return (*(DataPoint *)a).x - (*(DataPoint *)b).x;
}
static void sort_data(GooeyPlotData *data)
{
    if (!data || !data->x_data || !data->y_data || data->data_count == 0)
    {
        return;
    }

    DataPoint *points = calloc(data->data_count, sizeof(DataPoint));
    if (!points)
    {
        LOG_ERROR("Failed to allocate memory for sorting.");
        return;
    }

    for (size_t i = 0; i < data->data_count; ++i)
    {
        points[i].x = data->x_data[i];
        points[i].y = data->y_data[i];
    }

    qsort(points, data->data_count, sizeof(DataPoint), compare_data_points);

    for (size_t i = 0; i < data->data_count; ++i)
    {
        data->x_data[i] = points[i].x;
        data->y_data[i] = points[i].y;
    }
    free(points);
}
static void calculate_min_max_values(GooeyPlotData *data)
{
    data->max_x_value = -FLT_MAX;
    data->max_y_value = -FLT_MAX;
    data->min_x_value = FLT_MAX;
    data->min_y_value = FLT_MAX;
    if (!data || !data->x_data || !data->y_data)
    {
        return;
    }

    for (size_t j = 0; j < data->data_count; ++j)
    {
        if (data->x_data[j] > data->max_x_value)
            data->max_x_value = data->x_data[j];
        if (data->x_data[j] < data->min_x_value)
            data->min_x_value = data->x_data[j];
        if (data->y_data[j] > data->max_y_value)
            data->max_y_value = data->y_data[j];
        if (data->y_data[j] < data->min_y_value)
            data->min_y_value = data->y_data[j];
    }
}

static void add_placeholder_point(GooeyPlotData *data, bool initial)
{
    if (!data || data->data_count == 0)
    {
        return;
    }

    const float min_x_value_old = data->min_x_value;
    const float min_y_value_old = data->min_y_value;
    const float max_x_value_old = data->max_x_value;
    const float max_y_value_old = data->max_y_value;

    calculate_min_max_values(data);

    if (data->min_x_value < min_x_value_old || data->min_y_value < min_y_value_old ||
        data->max_x_value > max_x_value_old || data->max_y_value > max_y_value_old)
    {
        float *new_x_data = calloc((data->data_count + 1), sizeof(float));
        float *new_y_data = calloc((data->data_count + 1), sizeof(float));
        if (!new_x_data || !new_y_data)
        {
            LOG_ERROR("Failed to allocate memory for placeholder point.");
            free(new_x_data);
            free(new_y_data);
            return;
        }

        new_x_data[0] = data->min_x_value - data->x_step;
        new_y_data[0] = data->min_y_value - data->y_step;

        memcpy(&new_x_data[1], data->x_data, data->data_count * sizeof(float));
        memcpy(&new_y_data[1], data->y_data, data->data_count * sizeof(float));


        data->x_data = new_x_data;
        data->y_data = new_y_data;
        data->data_count += 1;

        data->min_x_value = new_x_data[0];
        data->min_y_value = new_y_data[0];
        data->max_x_value = (data->max_x_value > max_x_value_old) ? data->max_x_value : max_x_value_old;
        data->max_y_value = (data->max_y_value > max_y_value_old) ? data->max_y_value : max_y_value_old;
    }
}

GooeyPlot *GooeyPlot_Create(GOOEY_PLOT_TYPE plot_type, GooeyPlotData *data, int x, int y, int width, int height)
{
    if (!data)
    {
        LOG_ERROR("Invalid data provided.");
        return NULL;
    }

    GooeyPlot *plot = (GooeyPlot *) calloc(1, sizeof(GooeyPlot));
    
    if(!plot)
    {
        LOG_ERROR("Couldn't allocate memory for plot.");
        return NULL;
    }
    
    *plot = (GooeyPlot){0};
    
    plot->core.x = x;
    plot->core.y = y;
    plot->core.width = width;
    plot->core.height = height;
    plot->core.type = WIDGET_PLOT;
    plot->core.is_visible = true;
    plot->data = data;
    plot->data->plot_type = plot_type;
    plot->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);

    if (plot_type != GOOEY_PLOT_BAR)
        plot->data->bar_labels = NULL;

    if (plot->data)
    {

        plot->data->min_x_value = FLT_MAX;
        plot->data->min_y_value = FLT_MAX;
        plot->data->max_x_value = -FLT_MAX;
        plot->data->max_y_value = -FLT_MAX;

        add_placeholder_point(plot->data, true);
        sort_data(plot->data);
    }

    return plot;
}

void GooeyPlot_Update(GooeyPlot *plot, GooeyPlotData *new_data)
{
    if (!plot || !new_data)
    {
        LOG_ERROR("Invalid plot or data provided.");
        return;
    }

    plot->data = new_data;

    add_placeholder_point(plot->data, false);
    sort_data(plot->data);
}
#endif