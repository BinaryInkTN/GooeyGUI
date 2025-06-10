#include "gooey_layout_internal.h"
#if(ENABLE_LAYOUT)
#include "logger/pico_logger_internal.h"

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
        widget->is_visible = layout->core.is_visible;

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
#endif
