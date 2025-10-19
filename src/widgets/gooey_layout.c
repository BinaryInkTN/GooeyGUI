/*
 * Copyright (c) 2024 Yassine Ahmed Ali
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "widgets/gooey_layout.h"
#if(ENABLE_LAYOUT)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_window_internal.h"

GooeyLayout *GooeyLayout_Create(GooeyLayoutType layout_type,
                                int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        LOG_ERROR("Invalid layout dimensions: width=%d, height=%d", width, height);
        return NULL;
    }

    GooeyLayout *layout = (GooeyLayout *)calloc(1, sizeof(GooeyLayout));
    if (!layout)
    {
        LOG_ERROR("Failed to allocate memory for layout");
        return NULL;
    }

    *layout = (GooeyLayout){
        .core = {
            .type = WIDGET_LAYOUT,
            .sprite = active_backend->CreateSpriteForWidget(x, y, width, height),
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .is_visible = true,
            .disable_input = false
        },
        .layout_type = layout_type,
        .widget_count = 0,
        .widgets = {0}};

    return layout;
}

void GooeyLayout_AddChild(GooeyWindow* window, GooeyLayout *layout, void *widget)
{
    if (!layout)
    {
        LOG_ERROR("Null layout pointer");
        return;
    }

    if (!widget)
    {
        LOG_ERROR("Null widget pointer");
        return;
    }

    if (layout->widget_count >= MAX_WIDGETS)
    {
        LOG_ERROR("Maximum widget capacity (%d) reached", MAX_WIDGETS);
        return;
    }

    GooeyWidget *widget_core = (GooeyWidget *)widget;

    if (widget_core->type < WIDGET_LABEL || widget_core->type > WIDGET_TABS)
    {
        LOG_ERROR("Invalid widget type: %d", widget_core->type);
        return;
    }

    layout->widgets[layout->widget_count++] = widget_core;

    // Register widget to window implicitly
    GooeyWindow_Internal_RegisterWidget(window, widget);
}

void GooeyLayout_SetColumns(GooeyLayout *layout, int cols)
{
    if (!layout)
    {
        LOG_ERROR("Null layout pointer");
        return;
    }

    if (layout->layout_type != LAYOUT_GRID)
    {
        LOG_ERROR("Attempted to set columns on non-grid layout");
        return;
    }

    if (cols <= 0)
    {
        LOG_ERROR("Invalid number of columns: %d", cols);
        return;
    }

    layout->cols = cols;
}


void GooeyLayout_Destroy(GooeyLayout *layout)
{
    if (!layout)
    {
        LOG_WARNING("Attempted to destroy null layout");
        return;
    }

    // Note: This doesn't free child widgets - ownership must be managed separately
    free(layout);
}
#endif