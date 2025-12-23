#include "widgets/gooey_plot_internal.h"
#if (ENABLE_PLOT)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

#include "stdint.h"
#include "math.h"

#define VALUE_TICK_OFFSET 5
#define POINT_SIZE 10
#define PLOT_MARGIN 40
#define MIN_DATA_POINTS 2
#define MAX_TICK_COUNT 20
#define LABEL_BUFFER_SIZE 32
#define MAX_POINTS_FOR_DETAILED_DRAW 1000

typedef struct
{
    float x_step;
    float y_step;
    uint32_t x_tick_count;
    uint32_t y_tick_count;
    float x_value_spacing;
    float y_value_spacing;
    bool needs_recalculation;
    uint32_t last_data_count;
    float last_width;
    float last_height;
} PlotCache;

static PlotCache plot_cache = {0};

static void draw_plot_background(GooeyPlot *plot, GooeyWindow *win)
{
    if (!plot || !win)
        return;

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
    if (!plot || !win)
        return;

    int plot_x1 = plot->core.x + PLOT_MARGIN;
    int plot_x2 = plot->core.x + plot->core.width - PLOT_MARGIN;
    int plot_y1 = plot->core.y + PLOT_MARGIN;
    int plot_y2 = plot->core.y + plot->core.height - PLOT_MARGIN;

    active_backend->DrawLine(
        plot_x1, plot_y2, plot_x2, plot_y2,
        win->active_theme->neutral,
        win->creation_id, plot->core.sprite);

    active_backend->DrawLine(
        plot_x1, plot_y2, plot_x1, plot_y1,
        win->active_theme->neutral,
        win->creation_id, plot->core.sprite);
}

static void draw_plot_title(GooeyPlot *plot, GooeyWindow *win)
{
    if (!plot || !win || !plot->data->title)
        return;

    int text_width = active_backend->GetTextWidth(plot->data->title, strlen(plot->data->title));
    int title_x = plot->core.x + ((plot->core.width - text_width) / 2);
    int title_y = plot->core.y + PLOT_MARGIN / 2;

    active_backend->DrawGooeyText(
        title_x, title_y,
        plot->data->title,
        win->active_theme->primary,
        18.0f,
        win->creation_id, plot->core.sprite);
}

static void calculate_smart_ticks(float range, float custom_step, float *step, uint32_t *tick_count)
{

    if (custom_step > 0.0f)
    {
        *step = custom_step;
        *tick_count = (uint32_t)(range / custom_step) + 1;

        if (*tick_count < 2)
            *tick_count = 2;
        if (*tick_count > MAX_TICK_COUNT)
            *tick_count = MAX_TICK_COUNT;
        return;
    }

    if (range <= 0.0f)
    {
        *step = 1.0f;
        *tick_count = 2;
        return;
    }

    float rough_step = range / (MAX_TICK_COUNT - 1);
    float magnitude = powf(10.0f, floorf(log10f(rough_step)));
    float fraction = rough_step / magnitude;

    if (fraction < 1.5f)
    {
        *step = 1.0f * magnitude;
    }
    else if (fraction < 3.0f)
    {
        *step = 2.0f * magnitude;
    }
    else if (fraction < 7.0f)
    {
        *step = 5.0f * magnitude;
    }
    else
    {
        *step = 10.0f * magnitude;
    }

    *tick_count = (uint32_t)(range / *step) + 1;
    if (*tick_count < 2)
        *tick_count = 2;
    if (*tick_count > MAX_TICK_COUNT)
        *tick_count = MAX_TICK_COUNT;
}

static void draw_x_axis_ticks(GooeyPlot *plot, GooeyWindow *win, float min_value, float *plot_x_grid_coords)
{
    if (!plot || !win || !plot_x_grid_coords)
        return;

    float current_value = min_value;
    char value_str[LABEL_BUFFER_SIZE];

    // For bar plots with labels, don't show numeric x-axis labels
    bool show_numeric_labels = true;
    if (plot->data->plot_type == GOOEY_PLOT_BAR && plot->data->bar_labels != NULL) {
        show_numeric_labels = false;
    }

    for (uint32_t idx = 0; idx < plot_cache.x_tick_count; ++idx)
    {
        float x_pos = plot->core.x + PLOT_MARGIN + (plot_cache.x_value_spacing * idx);

        active_backend->DrawLine(
            x_pos, plot->core.y + plot->core.height - PLOT_MARGIN + VALUE_TICK_OFFSET,
            x_pos, plot->core.y + plot->core.height - PLOT_MARGIN - VALUE_TICK_OFFSET,
            win->active_theme->primary,
            win->creation_id, plot->core.sprite);

        plot_x_grid_coords[idx] = x_pos;

        // Only draw numeric labels if we're not using bar labels
        if (show_numeric_labels && (plot_cache.x_tick_count <= 15 || idx % 2 == 0))
        {

            if (fabsf(current_value) < 0.001f)
            {
                snprintf(value_str, sizeof(value_str), "0");
            }
            else if (fabsf(current_value) < 10.0f)
            {
                snprintf(value_str, sizeof(value_str), "%.1f", current_value);
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "%.0f", current_value);
            }

            int text_width = active_backend->GetTextWidth(value_str, strlen(value_str));
            float label_x = x_pos - (text_width / 2);
            float label_y = plot->core.y + plot->core.height - PLOT_MARGIN + VALUE_TICK_OFFSET + 15;

            active_backend->DrawGooeyText(
                label_x, label_y,
                value_str,
                win->active_theme->neutral,
               18.0f,
                win->creation_id, plot->core.sprite);
        }

        current_value += plot->data->x_step;
    }
}

static void draw_y_axis_ticks(GooeyPlot *plot, GooeyWindow *win, float min_value, float *plot_y_grid_coords)
{
    if (!plot || !win || !plot_y_grid_coords)
        return;

    float current_value = min_value;
    char value_str[LABEL_BUFFER_SIZE];

    for (uint32_t idx = 0; idx < plot_cache.y_tick_count; ++idx)
    {
        float y_pos = plot->core.y + plot->core.height - PLOT_MARGIN - (plot_cache.y_value_spacing * idx);

        active_backend->DrawLine(
            plot->core.x + PLOT_MARGIN - VALUE_TICK_OFFSET, y_pos,
            plot->core.x + PLOT_MARGIN + VALUE_TICK_OFFSET, y_pos,
            win->active_theme->primary,
            win->creation_id, plot->core.sprite);

        plot_y_grid_coords[idx] = y_pos;

        if (plot_cache.y_tick_count <= 10 || idx % 2 == 0)
        {

            if (fabsf(current_value) < 0.001f)
            {
                snprintf(value_str, sizeof(value_str), "0");
            }
            else if (fabsf(current_value) < 10.0f)
            {
                snprintf(value_str, sizeof(value_str), "%.1f", current_value);
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "%.0f", current_value);
            }

            int text_width = active_backend->GetTextWidth(value_str, strlen(value_str));
            float label_x = plot->core.x + PLOT_MARGIN - VALUE_TICK_OFFSET - text_width - 5;
            float label_y = y_pos - 8;

            active_backend->DrawGooeyText(
                label_x, label_y,
                value_str,
                win->active_theme->neutral,
                18.0f,
                win->creation_id, plot->core.sprite);
        }

        current_value += plot->data->y_step;
    }
}

static void draw_grid_lines(GooeyPlot *plot, GooeyWindow *win, float *plot_x_grid_coords, float *plot_y_grid_coords)
{
    if (!plot || !win || !plot_x_grid_coords || !plot_y_grid_coords)
        return;

    int plot_y_top = plot->core.y + PLOT_MARGIN;
    int plot_y_bottom = plot->core.y + plot->core.height - PLOT_MARGIN;
    int plot_x_left = plot->core.x + PLOT_MARGIN;
    int plot_x_right = plot->core.x + plot->core.width - PLOT_MARGIN;

    for (uint32_t i = 1; i < plot_cache.x_tick_count - 1; ++i)
    {
        active_backend->DrawLine(
            plot_x_grid_coords[i], plot_y_bottom,
            plot_x_grid_coords[i], plot_y_top,
            win->active_theme->base,
            win->creation_id, plot->core.sprite);
    }

    for (uint32_t i = 1; i < plot_cache.y_tick_count - 1; ++i)
    {
        active_backend->DrawLine(
            plot_x_left, plot_y_grid_coords[i],
            plot_x_right, plot_y_grid_coords[i],
            win->active_theme->base,
            win->creation_id, plot->core.sprite);
    }
}

static void normalize_data_points_fast(GooeyPlot *plot, float *plot_x_coords, float *plot_y_coords)
{
    if (!plot || !plot->data || !plot_x_coords || !plot_y_coords)
        return;

    float x_range = plot->data->max_x_value - plot->data->min_x_value;
    float y_range = plot->data->max_y_value - plot->data->min_y_value;

    if (x_range <= 0.0f)
        x_range = 1.0f;
    if (y_range <= 0.0f)
        y_range = 1.0f;

    float plot_width = plot->core.width - 2 * PLOT_MARGIN;
    float plot_height = plot->core.height - 2 * PLOT_MARGIN;

    float x_scale = plot_width / x_range;
    float y_scale = plot_height / y_range;
    float x_base = plot->core.x + PLOT_MARGIN - plot->data->min_x_value * x_scale;
    float y_base = plot->core.y + plot->core.height - PLOT_MARGIN + plot->data->min_y_value * y_scale;

    for (size_t j = 0; j < plot->data->data_count; ++j)
    {
        plot_x_coords[j] = x_base + plot->data->x_data[j] * x_scale;
        plot_y_coords[j] = y_base - plot->data->y_data[j] * y_scale;
    }
}

static void draw_line_plot_fast(GooeyPlot *plot, GooeyWindow *win, float *plot_x_coords, float *plot_y_coords)
{
    if (!plot || !win || !plot_x_coords || !plot_y_coords)
        return;

    if (plot->data->data_count > MAX_POINTS_FOR_DETAILED_DRAW)
    {

        size_t step = plot->data->data_count / MAX_POINTS_FOR_DETAILED_DRAW;
        if (step < 1)
            step = 1;

        for (size_t j = 0; j < plot->data->data_count - step; j += step)
        {
            size_t next_j = j + step;
            if (next_j >= plot->data->data_count)
                next_j = plot->data->data_count - 1;

            active_backend->DrawLine(
                (int)plot_x_coords[j], (int)plot_y_coords[j],
                (int)plot_x_coords[next_j], (int)plot_y_coords[next_j],
                win->active_theme->primary,
                win->creation_id, plot->core.sprite);
        }
    }
    else
    {

        for (size_t j = 0; j < plot->data->data_count - 1; ++j)
        {
            active_backend->DrawLine(
                (int)plot_x_coords[j], (int)plot_y_coords[j],
                (int)plot_x_coords[j + 1], (int)plot_y_coords[j + 1],
                win->active_theme->primary,
                win->creation_id, plot->core.sprite);
        }

        if (plot->data->data_count <= 100)
        {
            for (size_t j = 0; j < plot->data->data_count; ++j)
            {
                active_backend->FillRectangle(
                    (int)(plot_x_coords[j] - POINT_SIZE / 2),
                    (int)(plot_y_coords[j] - POINT_SIZE / 2),
                    POINT_SIZE, POINT_SIZE,
                    win->active_theme->primary,
                    win->creation_id, false, 0.0f, plot->core.sprite);
            }
        }
    }
}

static void draw_bar_plot_fast(GooeyPlot *plot, GooeyWindow *win, float *plot_x_coords, float *plot_y_coords)
{
    if (!plot || !win || !plot_x_coords || !plot_y_coords)
        return;

    const uint8_t bar_width = 30;
    const uint8_t LABEL_SPACING = 10;
    float base_y = plot->core.y + plot->core.height - PLOT_MARGIN;

    for (size_t j = 0; j < plot->data->data_count; ++j)
    {
        float bar_x = plot_x_coords[j] - (bar_width / 2);
        float bar_height = base_y - plot_y_coords[j];
        float bar_y = plot_y_coords[j];

        if (bar_height < 1.0f)
            bar_height = 1.0f;

        active_backend->FillRectangle(
            (int)bar_x, (int)bar_y,
            bar_width, (int)bar_height,
            win->active_theme->primary,
            win->creation_id, false, 0.0f, plot->core.sprite);

        active_backend->DrawRectangle(
            (int)bar_x, (int)bar_y,
            bar_width, (int)bar_height,
            win->active_theme->neutral, 1.0f,
            win->creation_id, false, 0.0f, plot->core.sprite);

        if (plot->data->data_count <= 20 && plot->data->bar_labels && plot->data->bar_labels[j])
        {
            const char *label = plot->data->bar_labels[j];
            float label_x = plot_x_coords[j] - active_backend->GetTextWidth(label, strlen(label)) / 2;
            float label_y = base_y + LABEL_SPACING;

            active_backend->DrawGooeyText(
                label_x, label_y,
                label,
                win->active_theme->neutral,
                18.0f,
                win->creation_id, plot->core.sprite);
        }
    }
}

static void draw_scatter_plot_fast(GooeyPlot *plot, GooeyWindow *win, float *plot_x_coords, float *plot_y_coords)
{
    if (!plot || !win || !plot_x_coords || !plot_y_coords)
        return;

    const uint8_t SCATTER_POINT_SIZE = plot->data->data_count > 500 ? 4 : 8;

    for (size_t j = 0; j < plot->data->data_count; ++j)
    {
        active_backend->FillRectangle(
            (int)(plot_x_coords[j] - SCATTER_POINT_SIZE / 2),
            (int)(plot_y_coords[j] - SCATTER_POINT_SIZE / 2),
            SCATTER_POINT_SIZE, SCATTER_POINT_SIZE,
            win->active_theme->primary,
            win->creation_id, false, 0.0f, plot->core.sprite);
    }
}

static void draw_data_points_optimized(GooeyPlot *plot, GooeyWindow *win, float *plot_x_coords, float *plot_y_coords)
{
    if (!plot || !win || !plot->data || !plot_x_coords || !plot_y_coords)
        return;

    normalize_data_points_fast(plot, plot_x_coords, plot_y_coords);

    switch (plot->data->plot_type)
    {
    case GOOEY_PLOT_LINE:
        draw_line_plot_fast(plot, win, plot_x_coords, plot_y_coords);
        break;

    case GOOEY_PLOT_BAR:
        draw_bar_plot_fast(plot, win, plot_x_coords, plot_y_coords);
        break;

    case GOOEY_PLOT_SCATTER:
        draw_scatter_plot_fast(plot, win, plot_x_coords, plot_y_coords);
        break;

    default:
        LOG_WARNING("Unknown plot type: %d", plot->data->plot_type);
        break;
    }
}

static bool needs_recalculation(GooeyPlot *plot)
{
    if (!plot || !plot->data)
        return true;

    return (plot_cache.last_data_count != plot->data->data_count ||
            plot_cache.last_width != plot->core.width ||
            plot_cache.last_height != plot->core.height ||
            plot_cache.needs_recalculation);
}

static void update_plot_cache(GooeyPlot *plot)
{
    if (!plot || !plot->data)
        return;

    float x_range = plot->data->max_x_value - plot->data->min_x_value;
    float y_range = plot->data->max_y_value - plot->data->min_y_value;

    if (x_range <= 0.0f)
        x_range = 1.0f;
    if (y_range <= 0.0f)
        y_range = 1.0f;

    calculate_smart_ticks(x_range, plot->data->custom_x_step, &plot_cache.x_step, &plot_cache.x_tick_count);
    calculate_smart_ticks(y_range, plot->data->custom_y_step, &plot_cache.y_step, &plot_cache.y_tick_count);

    plot->data->x_step = plot_cache.x_step;
    plot->data->y_step = plot_cache.y_step;

    plot_cache.x_value_spacing = (plot->core.width - 2 * PLOT_MARGIN) / (plot_cache.x_tick_count - 1);
    plot_cache.y_value_spacing = (plot->core.height - 2 * PLOT_MARGIN) / (plot_cache.y_tick_count - 1);

    plot_cache.last_data_count = plot->data->data_count;
    plot_cache.last_width = plot->core.width;
    plot_cache.last_height = plot->core.height;
    plot_cache.needs_recalculation = false;
}

void GooeyPlot_Draw(GooeyWindow *win)
{
    if (!win || win->plot_count == 0)
        return;

    for (size_t i = 0; i < win->plot_count; ++i)
    {
        GooeyPlot *plot = win->plots[i];
        if (!plot || !plot->data || !plot->core.is_visible)
            continue;

        if (!plot->data->x_data || !plot->data->y_data || plot->data->data_count < MIN_DATA_POINTS)
        {
            if (plot->data->data_count > 0)
            {
                LOG_WARNING("Invalid plot data: missing arrays or insufficient points");
            }
            continue;
        }

        if (needs_recalculation(plot))
        {
            update_plot_cache(plot);
        }

        float *plot_x_coords = malloc(plot->data->data_count * sizeof(float));
        float *plot_y_coords = malloc(plot->data->data_count * sizeof(float));
        float *plot_x_grid_coords = malloc(plot_cache.x_tick_count * sizeof(float));
        float *plot_y_grid_coords = malloc(plot_cache.y_tick_count * sizeof(float));

        if (!plot_x_coords || !plot_y_coords || !plot_x_grid_coords || !plot_y_grid_coords)
        {
            LOG_ERROR("Failed to allocate memory for plot coordinates");
            free(plot_x_coords);
            free(plot_y_coords);
            free(plot_x_grid_coords);
            free(plot_y_grid_coords);
            continue;
        }

        draw_plot_background(plot, win);
        draw_axes(plot, win);
        draw_plot_title(plot, win);

        draw_x_axis_ticks(plot, win, plot->data->min_x_value, plot_x_grid_coords);
        draw_y_axis_ticks(plot, win, plot->data->min_y_value, plot_y_grid_coords);
        draw_grid_lines(plot, win, plot_x_grid_coords, plot_y_grid_coords);
        draw_data_points_optimized(plot, win, plot_x_coords, plot_y_coords);

        free(plot_x_coords);
        free(plot_y_coords);
        free(plot_x_grid_coords);
        free(plot_y_grid_coords);
    }
}

void GooeyPlot_InvalidateCache(GooeyPlot *plot)
{
    if (plot)
    {
        plot_cache.needs_recalculation = true;
    }
}
#endif
