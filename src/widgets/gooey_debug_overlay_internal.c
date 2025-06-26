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
#include "widgets/gooey_debug_overlay_internal.h"
#if(ENABLE_DEBUG_OVERLAY)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "common/gooey_common.h"
#include "logger/pico_logger_internal.h"
#include <time.h>

#define OVERLAY_POS 20

#ifdef __linux__
static void get_memory_footprint(size_t *total_kb)
{
    *total_kb = 0;
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp)
        return;

    char line[256] = {0};
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            sscanf(line + 6, "%zu", total_kb); // Already in KB
            break;
        }
    }
    fclose(fp);
}
#elif defined(_WIN32)
static void get_memory_footprint(size_t *total_kb)
{
 //TODO: implement in GLPS
}
#else
static void get_memory_footprint(size_t *total_kb)
{
    *total_kb = 0;
}
#endif

static size_t get_window_widget_count(GooeyWindow *win)
{
    if (!win)
        return 0;

    return win->label_count +
           win->button_count +
           win->canvas_count +
           win->checkbox_count +
           win->drop_surface_count +
           win->dropdown_count +
           win->image_count +
           win->layout_count +
           win->list_count +
           win->plot_count +
           win->radio_button_group_count +
           win->slider_count +
           win->textboxes_count +
           win->progressbar_count +
           win->tab_count;
}

void GooeyDebugOverlay_Draw(GooeyWindow *win)
{
    if (!win || !win->enable_debug_overlay || !win->current_event)
        return;

    GooeyEvent *event = (GooeyEvent *)win->current_event;
    int window_width, window_height;
    active_backend->GetWinDim(&window_width, &window_height, win->creation_id);

    const int overlay_width = 300;
    const int overlay_height = 150;
    const int x_pos = window_width - overlay_width - 10;
    const int y_pos = window_height - overlay_height - 10;
    const int line_height = 18;
    const int padding = 18;

    active_backend->FillRectangle(x_pos, y_pos,
                                  overlay_width, overlay_height,
                                  0x3333AAAA, win->creation_id, false, 5.0f);

    int current_y = y_pos + padding;

    active_backend->DrawText(x_pos + padding, current_y, "DEBUG [GOOEY v1.0]",
                             win->active_theme->neutral, 0.3f, win->creation_id);
    current_y += line_height;

    char fps_text[64];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f",
             active_backend->GetWinFramerate(win->creation_id));
    active_backend->DrawText(x_pos + padding, current_y, fps_text,
                             win->active_theme->neutral, 0.27f, win->creation_id);
    current_y += line_height;

    char win_text[64];
    snprintf(win_text, sizeof(win_text), "Window: %dx%d",
             window_width, window_height);
    active_backend->DrawText(x_pos + padding, current_y, win_text,
                             win->active_theme->neutral, 0.27f, win->creation_id);
    current_y += line_height;

    char mouse_text[64];
    snprintf(mouse_text, sizeof(mouse_text), "Mouse: %d,%d",
             event->mouse_move.x, event->mouse_move.y);
    active_backend->DrawText(x_pos + padding, current_y, mouse_text,
                             win->active_theme->neutral, 0.27f, win->creation_id);
    current_y += line_height;

    size_t mem_kb = 0;
    get_memory_footprint(&mem_kb);
    char mem_text[64];
    snprintf(mem_text, sizeof(mem_text), "Memory: %.2f MB",
             (float)mem_kb / 1024.0f);
    active_backend->DrawText(x_pos + padding, current_y, mem_text,
                             win->active_theme->neutral, 0.27f, win->creation_id);
    current_y += line_height;

    char widget_text[64];
    snprintf(widget_text, sizeof(widget_text), "Widgets: %zu",
             get_window_widget_count(win));
    active_backend->DrawText(x_pos + padding, current_y, widget_text,
                             win->active_theme->neutral, 0.27f, win->creation_id);
    current_y += line_height;

    active_backend->DrawText(x_pos + padding, current_y, "Renderer: OpenGL [GLPS]",
                             win->active_theme->neutral, 0.27f, win->creation_id);
    current_y += line_height;

    time_t now = time(NULL);
    char time_text[64];
    strftime(time_text, sizeof(time_text), "%H:%M:%S", localtime(&now));
    active_backend->DrawText(x_pos + padding, current_y, time_text,
                             win->active_theme->neutral, 0.27f, win->creation_id);
}
#endif
