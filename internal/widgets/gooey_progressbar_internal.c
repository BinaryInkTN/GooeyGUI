/*
 * Copyright (c) 2025 Yassine Ahmed Ali
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

#include "gooey_progressbar_internal.h"
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include <string.h>

#define GOOEY_PROGRESSBAR_DEFAULT_RADIUS 5.0f

static void DrawProgressBarBackground(GooeyProgressBar *progressbar, GooeyWindow *win)
{
    active_backend->FillRectangle(
        progressbar->core.x,
        progressbar->core.y,
        progressbar->core.width,
        progressbar->core.height,
        win->active_theme->widget_base,
        win->creation_id, true, GOOEY_PROGRESSBAR_DEFAULT_RADIUS);
}

static void DrawProgressBarFill(GooeyProgressBar *progressbar, GooeyWindow *win)
{
    float fill_width = progressbar->core.width * ((float)progressbar->value / 100);
    fill_width = (fill_width < 0) ? 0 : fill_width;
    unsigned long color = win->active_theme->primary;
    if(progressbar->value < 50)
    {
        color = win->active_theme->danger;
    }
    else if(progressbar->value > 75)
    {
        color = win->active_theme->success;
    } else {
        color = win->active_theme->primary;
    }
    active_backend->FillRectangle(
        progressbar->core.x,
        progressbar->core.y,
        fill_width,
        progressbar->core.height,
        color,
        win->creation_id, true, GOOEY_PROGRESSBAR_DEFAULT_RADIUS);

   

    active_backend->DrawRectangle(
        progressbar->core.x,
        progressbar->core.y,
        progressbar->core.width,
        progressbar->core.height,
        color, 1.0f,
        win->creation_id, true, GOOEY_PROGRESSBAR_DEFAULT_RADIUS);
}

static void DrawProgressPercentage(GooeyProgressBar *progressbar, GooeyWindow *win)
{
    char display_percentage[64];
    snprintf(display_percentage, sizeof(display_percentage), "%ld%%", progressbar->value);

    float text_width = active_backend->GetTextWidth(display_percentage, strlen(display_percentage));
    float text_height = active_backend->GetTextHeight(display_percentage, strlen(display_percentage));

    float text_x = progressbar->core.x + (progressbar->core.width / 2) - (text_width / 2);
    float text_y = progressbar->core.y + (progressbar->core.height / 2) + (text_height / 2);

    active_backend->DrawText(
        text_x,
        text_y,
        display_percentage,
        win->active_theme->neutral,
        0.27f,
        win->creation_id);
}

void GooeyProgressBar_Draw(GooeyWindow *win)
{
    if (!win)
    {
        LOG_ERROR("Window is NULL");
        return;
    }

    if (!win->progressbars || win->progressbar_count == 0)
    {
        return;
    }

    for (size_t i = 0; i < win->progressbar_count; ++i)
    {
        GooeyProgressBar *progressbar = (GooeyProgressBar *)win->progressbars[i];
        if (!progressbar )
        {
            LOG_WARNING("Skipping NULL progress bar");
            continue;
        }

        if(!progressbar->core.is_visible)
        {
            continue;
        }

        DrawProgressBarBackground(progressbar, win);
        DrawProgressBarFill(progressbar, win);
        DrawProgressPercentage(progressbar, win);
    }
}