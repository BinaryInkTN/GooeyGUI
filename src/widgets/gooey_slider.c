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

#include "widgets/gooey_slider.h"
#include "core/gooey_backend_internal.h"

GooeySlider *GooeySlider_Add(GooeyWindow *win, int x, int y, int width,
                             long min_value, long max_value, bool show_hints,
                             void (*callback)(long value))
{
    if (!win)
    {
        LOG_ERROR("Window cannot be NULL. \n");
        return NULL;
    }

    if (max_value <= min_value)
    {
        LOG_ERROR("max_value should be greater than min_value. \n");
        return NULL;
    }

    win->sliders[win->slider_count] = (GooeySlider){0};
    GooeySlider *slider = &win->sliders[win->slider_count++];
    slider->core.type = WIDGET_SLIDER;
    slider->core.x = x;
    slider->core.y = y;
    slider->core.width = width;
    slider->core.height = SLIDER_HEIGHT;
    slider->min_value = min_value;
    slider->max_value = max_value;
    slider->value = min_value;
    slider->show_hints = show_hints;
    slider->callback = callback;
    GooeyWindow_RegisterWidget(win, (GooeyWidget *)&slider->core);

    return slider;
}

long GooeySlider_getValue(GooeySlider *slider)
{
    if (!slider)
    {
        LOG_ERROR("Widget<Slider> cannot be NULL. \n");
        return -1;
    }

    return slider->value;
}

void GooeySlider_setValue(GooeySlider *slider, long value)
{
    if (!slider)
    {
        LOG_ERROR("Widget<Slider> cannot be NULL. \n");
        return;
    }

    slider->value = value;
}

void GooeySlider_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->slider_count; ++i)
    {
        GooeySlider *slider = &win->sliders[i];

        active_backend->FillRectangle(slider->core.x,
                                      slider->core.y, slider->core.width, slider->core.height, active_theme->widget_base, win->creation_id);

        int thumb_x = slider->core.x + (slider->value - slider->min_value) *
                                           slider->core.width /
                                           (slider->max_value - slider->min_value);

        active_backend->FillRectangle(thumb_x - 5,
                                      slider->core.y - 5, 10, slider->core.height + 10, active_theme->primary, win->creation_id);

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
                min_value, active_theme->neutral, 0.25f, win->creation_id);
            active_backend->DrawText(
                slider->core.x + slider->core.width + 5, slider->core.y + 5,
                max_value, active_theme->neutral, 0.25f, win->creation_id);
            if (slider->value != 0)
                active_backend->DrawText(thumb_x - 5,
                                         slider->core.y + 25, value, active_theme->neutral, 0.25f, win->creation_id);
        }
        active_backend->SetForeground(active_theme->neutral);
    }
}
bool GooeySlider_HandleDrag(GooeyWindow *win, GooeyEvent *event)
{
    static GooeySlider *active_slider = NULL;
    const int comfort_margin = 20;

    int mouse_x = event->mouse_move.x;
    int mouse_y = event->mouse_move.y;

    if (event->type == GOOEY_EVENT_CLICK_RELEASE && active_slider)
    {

        if (active_slider->callback)
        {
            active_slider->callback(active_slider->value);
        }

        active_slider = NULL;
       // active_backend->InhibitResetEvents(0);

        return true;
    }

    for (size_t i = 0; i < win->slider_count; ++i)
    {
        GooeySlider *slider = &win->sliders[i];

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
       // active_backend->InhibitResetEvents(1);
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

    //active_backend->InhibitResetEvents(0);

    return false;
}
