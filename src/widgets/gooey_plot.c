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
#if (ENABLE_PLOT)
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
    DataPoint *pointA = (DataPoint *)a;
    DataPoint *pointB = (DataPoint *)b;

    if (pointA->x < pointB->x)
        return -1;
    if (pointA->x > pointB->x)
        return 1;
    return 0;
}

static void sort_data(GooeyPlotData *data)
{
    if (!data || !data->x_data || !data->y_data || data->data_count < 2)
    {
        return;
    }

    DataPoint *points = malloc(data->data_count * sizeof(DataPoint));
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
    if (!data || !data->x_data || !data->y_data || data->data_count == 0)
    {
        data->min_x_value = 0.0f;
        data->max_x_value = 1.0f;
        data->min_y_value = 0.0f;
        data->max_y_value = 1.0f;
        return;
    }

    data->max_x_value = data->x_data[0];
    data->max_y_value = data->y_data[0];
    data->min_x_value = data->x_data[0];
    data->min_y_value = data->y_data[0];

    for (size_t j = 1; j < data->data_count; ++j)
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

    if (data->max_x_value <= data->min_x_value)
    {
        data->max_x_value = data->min_x_value + 1.0f;
    }
    if (data->max_y_value <= data->min_y_value)
    {
        data->max_y_value = data->min_y_value + 1.0f;
    }
}

static float calculate_nice_step(float range, float custom_step)
{
    // Use custom step if provided and valid
    if (custom_step > 0.0f)
    {
        return custom_step;
    }

    // Fall back to automatic calculation
    if (range <= 0.0f)
        return 1.0f;

    float rough_step = range / 6.0f;
    float magnitude = powf(10.0f, floorf(log10f(rough_step)));
    float fraction = rough_step / magnitude;

    if (fraction < 1.5f)
        return 1.0f * magnitude;
    if (fraction < 3.0f)
        return 2.0f * magnitude;
    if (fraction < 7.0f)
        return 5.0f * magnitude;
    return 10.0f * magnitude;
}

static void calculate_step_sizes(GooeyPlotData *data)
{
    if (!data || data->data_count == 0)
    {
        data->x_step = 1.0f;
        data->y_step = 1.0f;
        return;
    }

    float x_range = data->max_x_value - data->min_x_value;
    float y_range = data->max_y_value - data->min_y_value;

    if (x_range <= 0.0f)
        x_range = 1.0f;
    if (y_range <= 0.0f)
        y_range = 1.0f;

    // Use custom steps if provided, otherwise calculate automatically
    data->x_step = calculate_nice_step(x_range, data->custom_x_step);
    data->y_step = calculate_nice_step(y_range, data->custom_y_step);

    if (data->x_step <= 0.0f)
        data->x_step = 1.0f;
    if (data->y_step <= 0.0f)
        data->y_step = 1.0f;
}

static void add_data_padding(GooeyPlotData *data)
{
    if (!data || data->data_count == 0)
    {
        return;
    }

    calculate_min_max_values(data);

    float x_range = data->max_x_value - data->min_x_value;
    float y_range = data->max_y_value - data->min_y_value;

    float x_padding = x_range * 0.05f;
    float y_padding = y_range * 0.05f;

    data->min_x_value -= x_padding;
    data->max_x_value += x_padding;
    data->min_y_value -= y_padding;
    data->max_y_value += y_padding;

    calculate_step_sizes(data);
}

GooeyPlot *GooeyPlot_Create(GOOEY_PLOT_TYPE plot_type, GooeyPlotData *data, int x, int y, int width, int height)
{
    if (!data)
    {
        LOG_ERROR("Invalid data provided.");
        return NULL;
    }

    if (data->data_count == 0)
    {
        LOG_WARNING("Creating plot with no data points.");
    }

    GooeyPlot *plot = (GooeyPlot *)calloc(1, sizeof(GooeyPlot));
    if (!plot)
    {
        LOG_ERROR("Couldn't allocate memory for plot.");
        return NULL;
    }

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

    if (plot->data && plot->data->data_count > 0)
    {
        if (plot_type == GOOEY_PLOT_LINE)
        {
            sort_data(plot->data);
        }

        calculate_min_max_values(plot->data);
        add_data_padding(plot->data);
    }
    else
    {
        plot->data->min_x_value = 0.0f;
        plot->data->max_x_value = 1.0f;
        plot->data->min_y_value = 0.0f;
        plot->data->max_y_value = 1.0f;
        plot->data->x_step = 1.0f;
        plot->data->y_step = 1.0f;
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

    if (plot->data->data_count > 0)
    {
        if (plot->data->plot_type == GOOEY_PLOT_LINE)
        {
            sort_data(plot->data);
        }

        calculate_min_max_values(plot->data);
        add_data_padding(plot->data);
    }
}

void GooeyPlot_SetCustomStep(GooeyPlot *plot, float x_step, float y_step)
{
    if (!plot || !plot->data)
    {
        LOG_ERROR("Invalid plot or data");
        return;
    }

    plot->data->custom_x_step = x_step;
    plot->data->custom_y_step = y_step;

    // Recalculate steps with custom values
    calculate_step_sizes(plot->data);
}
#endif