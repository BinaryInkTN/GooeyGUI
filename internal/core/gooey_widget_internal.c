/*
 Copyright (c) 2025 Yassine Ahmed Ali

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

#include "gooey_widget_internal.h"
#include "common/gooey_common.h"
#include "logger/pico_logger_internal.h"
#include <stdbool.h>


void GooeyWidget_MakeVisible_Internal(void* widget, bool state)
{
    if(!widget)
    {
        LOG_ERROR("Couldn't change widget visibility, widget is NULL.");
        return;
    }
    GooeyWidget *core = (GooeyWidget *) widget;
    core->is_visible = state;
}


void GooeyWidget_MoveTo_Internal(void* widget, int x, int y)
{
    if(!widget)
    {
        LOG_ERROR("Couldn't move widget, widget is NULL.");
        return;
    }

    

    GooeyWidget *core = (GooeyWidget *) widget;
    core->x = x < 0 ? core->x : x;
    core->y = y < 0 ? core->y : y;
}


void GooeyWidget_Resize_Internal(void* widget, int w, int h)
{
    if(!widget)
    {
        LOG_ERROR("Couldn't resize widget, widget is NULL.");
        return;
    }

    GooeyWidget *core = (GooeyWidget *) widget;
    core->width = w < 0 ? core->width : w;
    core->height = h < 0 ? core->height : h;
}