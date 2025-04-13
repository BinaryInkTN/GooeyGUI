#include "gooey_label_internal.h"
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

void GooeyLabel_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->label_count; ++i)
    {
        GooeyLabel* label = win->labels[i];
        GooeyWidget* widget = &label->core;  
        
        if (!widget->is_visible)
            continue;

        active_backend->DrawText(
            widget->x, 
            widget->y, 
            label->text, 
            label->is_using_custom_color ? label->color : win->active_theme->neutral, 
            label->font_size, 
            win->creation_id
        );
    }
}