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



#include "widgets/gooey_progressbar.h"
#if(ENABLE_PROGRESSBAR)
#include "logger/pico_logger_internal.h"

GooeyProgressBar *GooeyProgressBar_Create(int x, int y, int width, int height, long initial_value)
{
    GooeyProgressBar *progressbar = (GooeyProgressBar *)calloc(1, sizeof(GooeyProgressBar));

    if (!progressbar)
    {
        LOG_ERROR("Couldn't allocate memory to progressbar.");
        return NULL;
    }

    if(initial_value < 0 || initial_value > 100)
    {
        LOG_WARNING("Progress bar initial value exceeds limits [0, 100]");
        return NULL;
    }


    *progressbar = (GooeyProgressBar){
        .core.type = WIDGET_PROGRESSBAR,
        .core.x = x,
        .core.y = y,
        .core.is_visible = true,
        .core.width = width,
        .core.height = height,
        .value = initial_value};

    return progressbar;
}

void GooeyProgressBar_Update(GooeyProgressBar *progressbar, long new_value)
{

    if(!progressbar)
    {
        LOG_ERROR("progressbar widget is NULL.");
        return;
    }

    if(new_value < 0)
    {
        LOG_WARNING("Progress bar update value negative");
        return;
    }

    progressbar->value = new_value > 100 ? 100 : new_value;
}
#endif