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
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyLayout *GooeyLayout_Create(GooeyLayoutType layout_type,
                                int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        LOG_ERROR("Invalid layout dimensions: width=%d, height=%d", width, height);
        return NULL;
    }

    GooeyLayout *layout = (GooeyLayout *)malloc(sizeof(GooeyLayout));
    if (!layout)
    {
        LOG_ERROR("Failed to allocate memory for layout");
        return NULL;
    }

    *layout = (GooeyLayout){
        .core = {
            .type = WIDGET_LAYOUT,
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .is_visible = true},
        .layout_type = layout_type,
        .widget_count = 0,
        .widgets = {0}};

    return layout;
}

void GooeyLayout_AddChild(GooeyLayout *layout, void *widget)
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
}

void GooeyLayout_Build(GooeyLayout *layout)
{
    if (!layout)
    {
        LOG_ERROR("Null layout pointer");
        return;
    }

    if (layout->widget_count == 0)
    {
        LOG_WARNING("Layout has no widgets to arrange");
        return;
    }

    const int spacing = 30;
    int current_x = layout->core.x;
    int current_y = layout->core.y;

    // Calculate available width per widget
    const float max_widget_width = (layout->layout_type == LAYOUT_HORIZONTAL)
                                       ? (layout->core.width - (spacing * (layout->widget_count - 1))) / layout->widget_count
                                       : layout->core.width;

    for (int i = 0; i < layout->widget_count; i++)
    {
        GooeyWidget *widget = layout->widgets[i];
        if (!widget)
        {
            LOG_ERROR("Null widget at index %d", i);
            continue;
        }

        switch (layout->layout_type)
        {
        case LAYOUT_VERTICAL:
            if (widget->type != WIDGET_CHECKBOX && widget->type != WIDGET_IMAGE)
            {
                widget->width = max_widget_width;
            }
            widget->x = layout->core.x + (layout->core.width - widget->width) / 2;
            widget->y = current_y;
            current_y += widget->height + spacing;
            break;

        case LAYOUT_HORIZONTAL:
            if (widget->type != WIDGET_CHECKBOX && widget->type != WIDGET_IMAGE)
            {
                widget->width = max_widget_width;
            }
            widget->x = current_x;
            widget->y = layout->core.y + (layout->core.height - widget->height) / 2;
            current_x += widget->width + spacing;
            break;

        case LAYOUT_GRID:
            // Implement grid layout logic here
            LOG_WARNING("Grid layout not yet implemented");
            break;

        default:
            LOG_ERROR("Unsupported layout type: %d", layout->layout_type);
            return;
        }

        // Recursively build child layouts
        if (widget->type == WIDGET_LAYOUT)
        {
            GooeyLayout_Build((GooeyLayout *)widget);
        }
    }
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