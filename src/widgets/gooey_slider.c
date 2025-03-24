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
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeySlider *GooeySlider_Add( int x, int y, int width,
                             long min_value, long max_value, bool show_hints,
                             void (*callback)(long value))
{
 

    if (max_value <= min_value)
    {
        LOG_ERROR("max_value should be greater than min_value. \n");
        return NULL;
    }

    GooeySlider *slider = malloc(sizeof(GooeySlider));

    *slider = (GooeySlider){0};
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