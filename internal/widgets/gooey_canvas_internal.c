#include "gooey_canvas_internal.h"
#include "backends/gooey_backend_internal.h"

void GooeyCanvas_Draw(GooeyWindow *win)
{

    for (size_t i = 0; i < win->canvas_count; ++i)
    {
        GooeyCanvas *canvas = win->canvas[i];
        if (!canvas->core.is_visible)
            continue;
        for (int j = 0; j < canvas->element_count; ++j)
        {
            CanvaElement *element = &canvas->elements[j];
            switch (element->operation)
            {
            case CANVA_DRAW_RECT:
                CanvasDrawRectangleArgs *args = (CanvasDrawRectangleArgs *)element->args;
                if (args->is_filled)
                    active_backend->FillRectangle(args->x, args->y, args->width, args->height, args->color, win->creation_id, false, 0.0f);
                else
                    active_backend->DrawRectangle(args->x, args->y, args->width, args->height, args->color,1.0f, win->creation_id, false, 0.0f);
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