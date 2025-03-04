/*
 Copyright (c) 2024 Yassine Ahmed Ali

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

#include "widgets/gooey_layout.h"
#include "core/gooey_backend_internal.h"

GooeyLayout *GooeyLayout_Create(GooeyWindow *win, GooeyLayoutType layout_type,
                                int x, int y, int width, int height)
{
    if (!win || win->layout_count >= MAX_WIDGETS)
    {
        fprintf(stderr,
                "Window not initialized or unable to add more layouts (full).\n");
        return NULL;
    }
    win->layouts[win->layout_count] = (GooeyLayout) {0}; 
    GooeyLayout *layout = &win->layouts[win->layout_count++];

    layout->core.type = WIDGET_LAYOUT;
    layout->core.x = x;
    layout->core.y = y;
    layout->core.width = width;
    layout->core.height = height;
    layout->layout_type = layout_type;
    layout->widget_count = 0;

    return layout;
}
void GooeyLayout_AddChild(GooeyLayout *layout, void *widget)
{
    if (!layout)
    {
        LOG_ERROR("Error: Invalid layout pointer.\n");
        return;
    }

    if (!widget)
    {
        LOG_ERROR("Error: Invalid widget pointer.\n");
        return;
    }

    if (layout->widget_count >= MAX_WIDGETS)
    {
        LOG_ERROR("Error: Maximum widgets reached for the layout.\n");
        return;
    }
    layout->widgets[layout->widget_count++] = widget;
}
void GooeyLayout_Build(GooeyLayout *layout)
{
    if (!layout)
    {
        LOG_ERROR("Error: Invalid layout pointer.\n");
        return;
    }

    if (layout->widget_count == 0)
    {
        LOG_ERROR("Error: Layout has no widgets to arrange.\n");
        return;
    }

    int spacing = 30;
    int current_x = layout->core.x;
    int current_y = layout->core.y;

    float max_widget_width = layout->layout_type == LAYOUT_HORIZONTAL ? layout->core.width / layout->widget_count : layout->core.width;

    switch (layout->layout_type)
    {
    case LAYOUT_VERTICAL:
        for (int i = 0; i < layout->widget_count; i++)
        {
            GooeyWidget *widget = (GooeyWidget *)layout->widgets[i];
            if (!widget)
            {
                LOG_ERROR("Null widget encountered at index %d.\n", i);
                continue;
            }

            if (widget->type != WIDGET_CHECKBOX)
                widget->width = max_widget_width;
            widget->x = layout->core.x + (layout->core.width - widget->width) / 2;
            widget->y = current_y;
            if (widget->type == WIDGET_LAYOUT)
            {
                GooeyLayout_Build((GooeyLayout *)widget);
            }

            current_y += widget->height + spacing;
        }
        break;

    case LAYOUT_HORIZONTAL:
        for (int i = 0; i < layout->widget_count; i++)
        {
            GooeyWidget *widget = layout->widgets[i];
            widget->width = max_widget_width;
            widget->x = current_x;
            widget->y = layout->core.y + (layout->core.height - widget->height) / 2;
            if (widget->type == WIDGET_LAYOUT)
            {
                GooeyLayout_Build((GooeyLayout *)widget);
            }

            current_x += widget->width + spacing;
        }
        break;

    case LAYOUT_GRID:
    {
        break;
    }

    default:
        LOG_ERROR("Error: Unsupported layout type.\n");
        break;
    }
}