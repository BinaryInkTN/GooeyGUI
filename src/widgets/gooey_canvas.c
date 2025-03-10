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
#include "core/gooey_backend.h"

GooeyCanvas *GooeyCanvas_Create(int x, int y, int width,
                             int height)
{
    GooeyCanvas *canvas = (GooeyCanvas *) malloc(sizeof(GooeyButton));

    if(!canvas) 
    {
        LOG_ERROR("Couldn't allocated memory for canvas");
        return NULL;
    }

    *canvas = (GooeyCanvas) {0};
    canvas->core.type = WIDGET_CANVAS;
    canvas->core.x = x;
    canvas->core.y = y;
    canvas->core.width = width;
    canvas->core.height = height;
    canvas->elements = malloc(sizeof(CanvaElement) * 100);
    canvas->element_count = 0;
    LOG_INFO("Canvas added to window with dimensions x=%d, y=%d, w=%d, h=%d.", x, y, width, height);
    return canvas;
}

void GooeyCanvas_DrawRectangle(GooeyCanvas *canvas, int x, int y, int width, int height, unsigned long color_hex, bool is_filled)
{


    int x_win = x + canvas->core.x;
    int y_win = y + canvas->core.y;

    if (x_win >= canvas->core.x && x_win <= canvas->core.x + canvas->core.width && y_win >= canvas->core.y && y_win <= canvas->core.y + canvas->core.height)
    {
        CanvasDrawRectangleArgs *args = malloc(sizeof(CanvasDrawRectangleArgs));
        *args = (CanvasDrawRectangleArgs){.color = color_hex, .height = height, .width = width, .x = x_win, .y = y_win, .is_filled = is_filled};
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


    int x1_win = x1 + canvas->core.x;
    int y1_win = y1 + canvas->core.y;

    int x2_win = x2 + canvas->core.x;
    int y2_win = y2 + canvas->core.y;

    if (x1_win >= canvas->core.x && x1_win <= canvas->core.x + canvas->core.width && y1_win >= canvas->core.y && y1_win <= canvas->core.y + canvas->core.height && x2_win >= canvas->core.x && x2_win <= canvas->core.x + canvas->core.width && y2_win >= canvas->core.y && y2_win <= canvas->core.y + canvas->core.height)
    {
        CanvasDrawLineArgs *args = malloc(sizeof(CanvasDrawLineArgs));
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

    int x_win = x_center + canvas->core.x;
    int y_win = y_center + canvas->core.y;

    if (x_win + width >= canvas->core.x && x_win + width <= canvas->core.x + canvas->core.width && y_win + height >= canvas->core.y && y_win + height <= canvas->core.y + canvas->core.height)
    {
        CanvasDrawArcArgs *args = malloc(sizeof(CanvasDrawArcArgs));
        *args = (CanvasDrawArcArgs){.height = height, .width = width, .x_center = x_win, .y_center = y_win, .angle1 = angle1, .angle2 = angle2};
        canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_DRAW_ARC, .args = args};
        LOG_INFO("Drew arc with dimensions x_center=%d, y_center=%d, w=%d, h=%d in Canvas<x=%d, y=%d, w=%d, h=%d>.", x_center, y_center, width, height, canvas->core.x, canvas->core.y, canvas->core.width, canvas->core.height);
    }
    else
    {
        LOG_ERROR("Canvas<%d, %d>: Arc<%d, %d> with Dimensions<%d, %d> is out of boundaries. will be skipped. \n", canvas->core.x, canvas->core.y, x_center, y_center, width, height);
    }
}

void GooeyCanvas_SetForeground(GooeyCanvas *canvas, unsigned long color_hex)
{
    CanvasSetFGArgs *args = malloc(sizeof(CanvasSetFGArgs));
    *args = (CanvasSetFGArgs){.color = color_hex};
    canvas->elements[canvas->element_count++] = (CanvaElement){.operation = CANVA_DRAW_SET_FG, .args = args};
    LOG_INFO("Set foreground with color %lX.", color_hex);
}


void GooeyCanvas_Draw(GooeyWindow* win) {

    for (size_t i = 0; i < win->canvas_count; ++i)
    {
        for (int j = 0; j < win->canvas[i]->element_count; ++j)
        {
            CanvaElement *element = &win->canvas[i]->elements[j];
            switch (element->operation)
            {
            case CANVA_DRAW_RECT:
                CanvasDrawRectangleArgs *args = (CanvasDrawRectangleArgs *)element->args;
                if (args->is_filled)
                    active_backend->FillRectangle(args->x, args->y, args->width, args->height, args->color, win->creation_id);
                else
                    active_backend->DrawRectangle(args->x, args->y, args->width, args->height, args->color, win->creation_id);
                break;

            case CANVA_DRAW_LINE:
                CanvasDrawLineArgs *args_line = (CanvasDrawLineArgs *)element->args;
                active_backend->DrawLine(args_line->x1, args_line->y1, args_line->x2, args_line->y2, args_line->color, win->creation_id);
                break;
            case CANVA_DRAW_ARC:
                CanvasDrawArcArgs *args_arc = (CanvasDrawArcArgs *)element->args;
                active_backend->FillArc(args_arc->x_center, args_arc->y_center, args_arc->width, args_arc->height, args_arc->angle1, args_arc->angle2, win->creation_id);
                break;
            case CANVA_DRAW_SET_FG:
                CanvasSetFGArgs *args_fg = (CanvasSetFGArgs *)element->args;
                active_backend->SetForeground(args_fg->color);
                break;
            default:
                break;
            }
        }
    }

}