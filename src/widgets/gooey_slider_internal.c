#include "widgets/gooey_slider_internal.h"
#if (ENABLE_SLIDER)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"

#define GOOEY_SLIDER_DEFAULT_RADIUS 2.0f
void GooeySlider_Draw(GooeyWindow *win)
{

    for (size_t i = 0; i < win->slider_count; ++i)
    {
        GooeySlider *slider = win->sliders[i];
        if (!slider || !slider->core.is_visible)
            continue;
        active_backend->FillRectangle(slider->core.x,
                                      slider->core.y, slider->core.width, slider->core.height, win->active_theme->widget_base, win->creation_id, false, 0.0f, slider->core.sprite);

        int thumb_x = slider->core.x + (slider->value - slider->min_value) *
                                           slider->core.width /
                                           (slider->max_value - slider->min_value);

        active_backend->FillRectangle(thumb_x - 5,
                                      slider->core.y - 5, 10, slider->core.height + 10, win->active_theme->primary, win->creation_id, true, GOOEY_SLIDER_DEFAULT_RADIUS, slider->core.sprite);

        active_backend->FillRectangle(slider->core.x,
                                      slider->core.y, thumb_x - slider->core.x, slider->core.height, win->active_theme->primary, win->creation_id, true, GOOEY_SLIDER_DEFAULT_RADIUS, slider->core.sprite);

        if (slider->show_hints)
        {

            char min_value[20];
            char max_value[20];
            char value[20];
            sprintf(min_value, "%ld", slider->min_value);
            sprintf(max_value, "%ld", slider->max_value);
            sprintf(value, "%ld", slider->value);
            int min_value_width = active_backend->GetTextWidth(min_value, strlen(min_value));
            int max_value_width = active_backend->GetTextWidth(max_value, strlen(max_value));
            int value_width = active_backend->GetTextWidth(value, strlen(value));

            active_backend->DrawText(
                slider->core.x - min_value_width - 5, slider->core.y + 5,
                min_value, win->active_theme->neutral, 18.0f, win->creation_id, slider->core.sprite);
            active_backend->DrawText(
                slider->core.x + slider->core.width + 5, slider->core.y + 5,
                max_value, win->active_theme->neutral, 18.0f, win->creation_id, slider->core.sprite);
            if (slider->value != 0)
                active_backend->DrawText(thumb_x - 5,
                                         slider->core.y + 25, value, win->active_theme->neutral, 18.0f, win->creation_id,  slider->core.sprite);
        }
        active_backend->SetForeground(win->active_theme->neutral);
        if (slider->core.sprite && slider->core.sprite->needs_redraw)
            active_backend->ResetRedrawSprite(slider->core.sprite);
    }
}
bool GooeySlider_HandleDrag(GooeyWindow *win, void *drag_event)
{
    static GooeySlider *active_slider = NULL;
    const int comfort_margin = 20;
    GooeyEvent *event = (GooeyEvent *)drag_event;
    int mouse_x = event->mouse_move.x;
    int mouse_y = event->mouse_move.y;

    if (event->type == GOOEY_EVENT_CLICK_RELEASE && active_slider)
    {

        if (active_slider->callback)
        {
            active_slider->callback(active_slider->value, active_slider->user_data);
        }

        active_slider = NULL;
        return false;
    }

    for (size_t i = 0; i < win->slider_count; ++i)
    {
        GooeySlider *slider = win->sliders[i];
        if (!slider || !slider->core.is_visible || slider->core.disable_input)
            continue;
        bool within_bounds =
            (mouse_y >= slider->core.y - comfort_margin && mouse_y <= slider->core.y + slider->core.height + comfort_margin) &&
            (mouse_x >= slider->core.x && mouse_x <= slider->core.x + slider->core.width);

        if (within_bounds && event->type == GOOEY_EVENT_CLICK_PRESS)
        {

            active_slider = slider;
        }
    }

    if (active_slider)
    {
        active_backend->RedrawSprite(active_slider->core.sprite);
        active_slider->value =
            active_slider->min_value +
            ((mouse_x - active_slider->core.x) * (active_slider->max_value - active_slider->min_value)) /
                active_slider->core.width;

        if (active_slider->value < active_slider->min_value)
            active_slider->value = active_slider->min_value;
        if (active_slider->value > active_slider->max_value)
            active_slider->value = active_slider->max_value;

        return true;
    }

    return false;
}
#endif