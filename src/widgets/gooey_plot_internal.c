#include "widgets/gooey_plot_internal.h"
#if(ENABLE_PLOT)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

#include "stdint.h"
#define VALUE_TICK_OFFSET 5

#define POINT_SIZE 10
#define PLOT_MARGIN 40

static void draw_plot_background(GooeyPlot *plot, GooeyWindow *win)
{

    active_backend->FillRectangle(
        plot->core.x,
        plot->core.y,
        plot->core.width,
        plot->core.height,
        win->active_theme->widget_base,
        win->creation_id, false, 0.0f, plot->core.sprite);
}

static void draw_axes(GooeyPlot *plot, GooeyWindow *win)
{

    // Draw the X axis
    active_backend->DrawLine(
        plot->core.x + PLOT_MARGIN,
        plot->core.y + plot->core.height - PLOT_MARGIN,
        plot->core.x + plot->core.width - PLOT_MARGIN,
        plot->core.y + plot->core.height - PLOT_MARGIN,
        win->active_theme->neutral,
        win->creation_id, plot->core.sprite);

    // Draw the Y axis
    active_backend->DrawLine(
        plot->core.x + PLOT_MARGIN,
        plot->core.y + plot->core.height - PLOT_MARGIN,
        plot->core.x + PLOT_MARGIN,
        plot->core.y + PLOT_MARGIN,
        win->active_theme->neutral,
        win->creation_id, plot->core.sprite);
}

static void draw_plot_title(GooeyPlot *plot, GooeyWindow *win)
{

    if (!plot->data->title)
        return;

    active_backend->DrawText(
        plot->core.x + ((plot->core.width / 2) - (active_backend->GetTextWidth(plot->data->title, strlen(plot->data->title)) / 2)),
        plot->core.y + PLOT_MARGIN / 2,
        plot->data->title,
        win->active_theme->primary,
        0.28f,
        win->creation_id, plot->core.sprite);
}

static void draw_x_axis_ticks(GooeyPlot *plot, GooeyWindow *win, float min_x_value, float x_value_spacing, uint32_t x_tick_count, float *plot_x_grid_coords)
{

    float x_default_value = ceilf(plot->data->x_data[0]);
    for (size_t idx = 0; idx < x_tick_count; ++idx)
    {
        if (idx != 0)
        {
            active_backend->DrawLine(
                plot->core.x + PLOT_MARGIN + x_value_spacing * idx,
                plot->core.y + plot->core.height - PLOT_MARGIN + VALUE_TICK_OFFSET,
                plot->core.x + PLOT_MARGIN + x_value_spacing * idx,
                plot->core.y + plot->core.height - PLOT_MARGIN - VALUE_TICK_OFFSET,
                win->active_theme->primary,
                win->creation_id, plot->core.sprite);

            plot_x_grid_coords[idx - 1] = plot->core.x + PLOT_MARGIN + x_value_spacing * idx;
        }

        char x_value_str[64];
        snprintf(x_value_str, sizeof(x_value_str), "%.2f", x_default_value);
        active_backend->DrawText(
            plot->core.x + PLOT_MARGIN + x_value_spacing * idx,
            plot->core.y + plot->core.height - PLOT_MARGIN + VALUE_TICK_OFFSET + 15,
            x_value_str,
            win->active_theme->neutral,
            0.28f,
            win->creation_id,plot->core.sprite);

        x_default_value += plot->data->x_step;
    }
}

static void draw_y_axis_ticks(GooeyPlot *plot, GooeyWindow *win, float min_y_value, float y_value_spacing, uint32_t y_tick_count, float *plot_y_grid_coords)
{

    float y_default_value = ceil(plot->data->y_data[0]);
    for (size_t idx = 0; idx < y_tick_count; ++idx)
    {
        if (idx != 0)
        {
            active_backend->DrawLine(
                plot->core.x + PLOT_MARGIN - VALUE_TICK_OFFSET,
                plot->core.y + plot->core.height - PLOT_MARGIN - y_value_spacing * idx,
                plot->core.x + PLOT_MARGIN + VALUE_TICK_OFFSET,
                plot->core.y + plot->core.height - PLOT_MARGIN - y_value_spacing * idx,
                win->active_theme->primary,
                win->creation_id, plot->core.sprite);

            plot_y_grid_coords[idx - 1] = plot->core.y + plot->core.height - PLOT_MARGIN - y_value_spacing * idx;
        }

        char y_value_str[64];
        snprintf(y_value_str, sizeof(y_value_str), "%.2f", y_default_value);
        active_backend->DrawText(
            plot->core.x,
            plot->core.y + plot->core.height - PLOT_MARGIN - y_value_spacing * idx,
            y_value_str,
            win->active_theme->neutral,
            0.28f,
            win->creation_id, plot->core.sprite);

        y_default_value += plot->data->y_step;
    }
}

static void draw_grid_lines(GooeyPlot *plot, GooeyWindow *win, uint32_t x_tick_count, uint32_t y_tick_count, float *plot_x_grid_coords, float *plot_y_grid_coords)
{

    for (size_t i = 0; i < x_tick_count - 1; ++i)
    {
        active_backend->DrawLine(
            plot_x_grid_coords[i],
            plot->core.y + plot->core.height - PLOT_MARGIN,
            plot_x_grid_coords[i],
            plot->core.y + PLOT_MARGIN,
            win->active_theme->base,
            win->creation_id, plot->core.sprite);
    }

    for (size_t i = 0; i < y_tick_count - 1; ++i)
    {
        active_backend->DrawLine(
            plot->core.x + PLOT_MARGIN,
            plot_y_grid_coords[i],
            plot->core.x + plot->core.width - PLOT_MARGIN,
            plot_y_grid_coords[i],
            win->active_theme->base,
            win->creation_id, plot->core.sprite);
    }
}

static void draw_data_points(GooeyPlot *plot, GooeyWindow *win, float min_x_value, float min_y_value, uint32_t x_tick_count, uint32_t y_tick_count, float *plot_x_coords, float *plot_y_coords)
{

    for (size_t j = 0; j < plot->data->data_count; ++j)
    {
        float x_axis_length = ((x_tick_count - 1) * plot->data->x_step);
        float y_axis_length = ((y_tick_count - 1) * plot->data->y_step);

        float normalized_x = (float)(plot->data->x_data[j] - min_x_value) / x_axis_length;
        float normalized_y = (float)(plot->data->y_data[j] - min_y_value) / y_axis_length;
        plot_x_coords[j] = plot->core.x + PLOT_MARGIN + normalized_x * (plot->core.width - 2 * PLOT_MARGIN);
        plot_y_coords[j] = plot->core.y + plot->core.height - PLOT_MARGIN - normalized_y * (plot->core.height - 2 * PLOT_MARGIN);
    }

    switch (plot->data->plot_type)
    {
    case GOOEY_PLOT_LINE:
    {
        for (size_t j = 0; j < plot->data->data_count - 1; ++j)
        {

            active_backend->FillRectangle(
                (int)plot_x_coords[j + 1] - POINT_SIZE / 2,
                (int)plot_y_coords[j + 1] - POINT_SIZE / 2,
                POINT_SIZE,
                POINT_SIZE,
                win->active_theme->primary,
                win->creation_id, false, 0.0f, plot->core.sprite);

            if (j < plot->data->data_count - 2)
            {
                active_backend->DrawLine(
                    (int)plot_x_coords[j + 1],
                    (int)plot_y_coords[j + 1],
                    (int)plot_x_coords[j + 2],
                    (int)plot_y_coords[j + 2],
                    win->active_theme->primary,
                    win->creation_id, plot->core.sprite);
            }
        }
        break;
    }

    case GOOEY_PLOT_BAR:

    {
        const uint8_t bar_width = 30;

        // todo: fix the first value on the bar plot.
        for (size_t j = 0; j < plot->data->data_count; ++j)
        {
            if (j != 0)
            {
                float bar_x = plot_x_coords[j] - ((float)bar_width / 2);
                float bar_height = (float)(plot->core.y + plot->core.height - PLOT_MARGIN) - plot_y_coords[j];
                float bar_y = plot_y_coords[j];

                active_backend->FillRectangle(
                    (int)bar_x,
                    (int)bar_y,
                    bar_width,
                    (int)bar_height,
                    win->active_theme->primary,
                    win->creation_id, false, 0.0f, plot->core.sprite);

                const char *label = plot->data->bar_labels[j - 1];
                const uint8_t LABEL_SPACING = 10;
                float label_x = plot_x_coords[j] - active_backend->GetTextWidth(label, strlen(label)) / 2;
                float label_y = (plot->core.y + plot->core.height - PLOT_MARGIN) + LABEL_SPACING;

                active_backend->DrawText(
                    label_x,
                    label_y,
                    label,
                    win->active_theme->neutral,
                    0.28f,
                    win->creation_id, plot->core.sprite);
            }
        }
        break;
    }
    case GOOEY_PLOT_SCATTER:
    {
        for (size_t j = 0; j < plot->data->data_count - 1; ++j)
        {
            const uint8_t SCATTER_POINT_SIZE = 10;
            active_backend->FillRectangle(
                (int)plot_x_coords[j + 1],
                (int)plot_y_coords[j + 1],
                (int)SCATTER_POINT_SIZE,
                (int)SCATTER_POINT_SIZE,
                win->active_theme->primary,
                win->creation_id, false, 0.0f, plot->core.sprite);
        }
        break;
    }
    default:
        break;
    }
}

void GooeyPlot_Draw(GooeyWindow *win)
{

    if (!win || win->plot_count == 0)
    {
        return;
    }

    for (size_t i = 0; i < win->plot_count; ++i)
    {
        GooeyPlot *plot = win->plots[i];
        if (!plot || !plot->data || !plot->data->x_data || !plot->data->y_data || !plot->core.is_visible)
        {
            continue;
        }

        float x_range = plot->data->max_x_value - plot->data->min_x_value;
        float y_range = plot->data->max_y_value - plot->data->min_y_value;
        if (x_range == 0)
            x_range = 1;
        if (y_range == 0)
            y_range = 1;

        uint32_t x_tick_count = (uint32_t)(ceilf(x_range / plot->data->x_step)) + 1;
        uint32_t y_tick_count = (uint32_t)(ceilf(y_range / plot->data->y_step)) + 1;
        float *plot_x_coords = malloc(plot->data->data_count * sizeof(float));
        float *plot_y_coords = malloc(plot->data->data_count * sizeof(float));
        float *plot_x_grid_coords = malloc((x_tick_count - 1) * sizeof(float));
        float *plot_y_grid_coords = malloc((y_tick_count - 1) * sizeof(float));

        if (!plot_x_coords || !plot_y_coords || !plot_x_grid_coords || !plot_y_grid_coords)
        {
            LOG_ERROR("Failed to allocate memory for plot coordinates.");
            free(plot_x_coords);
            free(plot_y_coords);
            free(plot_x_grid_coords);
            free(plot_y_grid_coords);
            continue;
        }

        draw_plot_background(plot, win);
        draw_axes(plot, win);
        draw_plot_title(plot, win);

        float x_value_spacing = (plot->core.width - 2 * PLOT_MARGIN) / (x_tick_count - 1);
        draw_x_axis_ticks(plot, win, plot->data->min_x_value, x_value_spacing, x_tick_count, plot_x_grid_coords);

        float y_value_spacing = (plot->core.height - 2 * PLOT_MARGIN) / (y_tick_count - 1);
        draw_y_axis_ticks(plot, win, plot->data->min_y_value, y_value_spacing, y_tick_count, plot_y_grid_coords);

        draw_grid_lines(plot, win, x_tick_count, y_tick_count, plot_x_grid_coords, plot_y_grid_coords);
        draw_data_points(plot, win, plot->data->min_x_value, plot->data->min_y_value, x_tick_count, y_tick_count, plot_x_coords, plot_y_coords);

        free(plot_x_coords);
        free(plot_y_coords);
        free(plot_x_grid_coords);
        free(plot_y_grid_coords);
    }
}
#endif
