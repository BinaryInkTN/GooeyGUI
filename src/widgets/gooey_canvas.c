

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

#include "widgets/gooey_canvas.h"
#if (ENABLE_CANVAS)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyCanvas *GooeyCanvas_Create(int x, int y, int width,
                                int height, void (*callback)(int x, int y, void *user_data), void *user_data)
{
    GooeyCanvas *canvas = (GooeyCanvas *)calloc(1, sizeof(GooeyButton));

    if (!canvas)
    {
        LOG_ERROR("Couldn't allocate memory for canvas");
        return NULL;
    }

    *canvas = (GooeyCanvas){0};
    canvas->core.type = WIDGET_CANVAS;
    canvas->core.x = x;
    canvas->core.y = y;
    canvas->core.width = width;
    canvas->core.height = height;
    canvas->core.is_visible = true;
    canvas->elements = calloc(100, sizeof(CanvaElement));
    canvas->element_count = 0;
    canvas->callback = callback;
    canvas->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);
    canvas->user_data = user_data;
    canvas->core.disable_input = false;

    LOG_INFO("Canvas added to window with dimensions x=%d, y=%d, w=%d, h=%d.", x, y, width, height);
    return canvas;
}

void GooeyCanvas_DrawRectangle(GooeyCanvas *canvas, int x, int y, int width, int height, unsigned long color_hex, bool is_filled, float thickness, bool is_rounded, float corner_radius)
{
    if (!canvas)
    {
        LOG_ERROR("Couldn't allocate memory for canvas");
        return;
    }

    int x_win = x + canvas->core.x;
    int y_win = y + canvas->core.y;

    if (x_win >= canvas->core.x && x_win <= canvas->core.x + canvas->core.width && y_win >= canvas->core.y && y_win <= canvas->core.y + canvas->core.height)
    {
        CanvasDrawRectangleArgs *args = calloc(1, sizeof(CanvasDrawRectangleArgs));
        *args = (CanvasDrawRectangleArgs){.color = color_hex, .height = height, .width = width, .x = x_win, .y = y_win, .is_filled = is_filled, .thickness = thickness, .is_rounded = is_rounded, .corner_radius = corner_radius};
        canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_DRAW_RECT, .args = args};
        LOG_INFO("Drew %s rectangle with dimensions x=%d, y=%d, w=%d, h=%d in canvas<x=%d, y=%d, w=%d, h=%d>.", is_filled ? "filled" : "hollow", x, y, width, height, canvas->core.x, canvas->core.y, canvas->core.width, canvas->core.height);
    }
    else
    {
        LOG_ERROR("Canvas<%d, %d>: Rectangle<%d, %d> is out of boundaries. will be skipped. \n", canvas->core.x, canvas->core.y, x, y);
    }
}

void GooeyCanvas_DrawLine(GooeyCanvas *canvas, int x1, int y1, int x2, int y2, unsigned long color_hex)
{
    if (!canvas)
    {
        LOG_ERROR("Couldn't allocate memory for canvas");
        return;
    }
    int x1_win = x1 + canvas->core.x;
    int y1_win = y1 + canvas->core.y;

    int x2_win = x2 + canvas->core.x;
    int y2_win = y2 + canvas->core.y;

    if (x1_win >= canvas->core.x && x1_win <= canvas->core.x + canvas->core.width && y1_win >= canvas->core.y && y1_win <= canvas->core.y + canvas->core.height && x2_win >= canvas->core.x && x2_win <= canvas->core.x + canvas->core.width && y2_win >= canvas->core.y && y2_win <= canvas->core.y + canvas->core.height)
    {
        CanvasDrawLineArgs *args = calloc(1, sizeof(CanvasDrawLineArgs));
        *args = (CanvasDrawLineArgs){.color = color_hex, .x1 = x1_win, .x2 = x2_win, .y1 = y1_win, .y2 = y2_win};
        canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_DRAW_LINE, .args = args};
        LOG_INFO("Drew line with dimensions x1=%d, y1=%d, x2=%d, y2=%d", x1, y1, x2, y2);
    }
    else
    {
        LOG_ERROR("Canvas<%d, %d>: Line<%d, %d, %d, %d> is out of boundaries. will be skipped. \n", canvas->core.x, canvas->core.y, x1, y1, x2, y2);
    }
}

void GooeyCanvas_DrawArc(GooeyCanvas *canvas, int x_center, int y_center, int width, int height, int angle1, int angle2)
{
    if (!canvas)
    {
        LOG_ERROR("Couldn't allocate memory for canvas");
        return;
    }
    int x_win = x_center + canvas->core.x;
    int y_win = y_center + canvas->core.y;

    if (x_win + width >= canvas->core.x && x_win + width <= canvas->core.x + canvas->core.width && y_win + height >= canvas->core.y && y_win + height <= canvas->core.y + canvas->core.height)
    {
        CanvasDrawArcArgs *args = calloc(1, sizeof(CanvasDrawArcArgs));
        *args = (CanvasDrawArcArgs){.height = height, .width = width, .x_center = x_win, .y_center = y_win, .angle1 = angle1, .angle2 = angle2};
        canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_DRAW_ARC, .args = args};
        LOG_INFO("Drew arc with dimensions x_center=%d, y_center=%d, w=%d, h=%d in Canvas<x=%d, y=%d, w=%d, h=%d>.", x_center, y_center, width, height, canvas->core.x, canvas->core.y, canvas->core.width, canvas->core.height);
    }
    else
    {
        LOG_ERROR("Canvas<%d, %d>: Arc<%d, %d> with Dimensions<%d, %d> is out of boundaries. will be skipped. \n", canvas->core.x, canvas->core.y, x_center, y_center, width, height);
    }
}

void GooeyCanvas_Clear(GooeyCanvas *canvas)
{
    if (!canvas)
    {
        LOG_ERROR("Couldn't allocate memory for canvas");
        return;
    }

    for (int i = 0; i < canvas->element_count; i++)
    {
        if (canvas->elements[i].args)
        {
            free(canvas->elements[i].args);
        }
    }

    canvas->element_count = 0;

    CanvasClearArgs *args = calloc(1, sizeof(CanvasClearArgs));
    *args = (CanvasClearArgs){0};
    canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_CLEAR, .args = args};

    LOG_INFO("Canvas cleared. All previous elements removed.");
}
void GooeyCanvas_SetForeground(GooeyCanvas *canvas, unsigned long color_hex)
{
    CanvasSetFGArgs *args = calloc(1, sizeof(CanvasSetFGArgs));
    *args = (CanvasSetFGArgs){.color = color_hex};
    canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_DRAW_SET_FG, .args = args};
    LOG_INFO("Set foreground with color %lX.", color_hex);
}
#endif