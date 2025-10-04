
#include "widgets/gooey_canvas_internal.h"
#if (ENABLE_CANVAS)

#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

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
            {
                CanvasDrawRectangleArgs *args = (CanvasDrawRectangleArgs *)element->args;
                if (args->is_filled)
                    active_backend->FillRectangle(args->x, args->y, args->width, args->height, args->color, win->creation_id, args->is_rounded, args->corner_radius, canvas->core.sprite);
                else
                    active_backend->DrawRectangle(args->x, args->y, args->width, args->height, args->color, args->thickness, win->creation_id, args->is_rounded, args->corner_radius, canvas->core.sprite);
                break;
            }

            case CANVA_DRAW_LINE:
            {
                CanvasDrawLineArgs *args_line = (CanvasDrawLineArgs *)element->args;
                active_backend->DrawLine(args_line->x1, args_line->y1, args_line->x2, args_line->y2, args_line->color, win->creation_id, canvas->core.sprite);
                break;
            }

            case CANVA_DRAW_ARC:
            {
                CanvasDrawArcArgs *args_arc = (CanvasDrawArcArgs *)element->args;
                active_backend->FillArc(args_arc->x_center, args_arc->y_center, args_arc->width, args_arc->height, args_arc->angle1, args_arc->angle2, win->creation_id, canvas->core.sprite);
                break;
            }

            case CANVA_DRAW_SET_FG:
            {
                CanvasSetFGArgs *args_fg = (CanvasSetFGArgs *)element->args;
                active_backend->SetForeground(args_fg->color);
                break;
            }

            default:
                break;
            }
        }
    }
}

void GooeyCanvas_HandleClick(GooeyWindow *win, int x, int y)
{
    if (!win)
    {
        LOG_ERROR("Window invalid.");
        return;
    }

    for (size_t i = 0; i < win->canvas_count; ++i)
    {
        GooeyCanvas *canvas = win->canvas[i];
        if (!canvas)
        {
            LOG_ERROR("Canvas is invalid.");
            continue;
        }

        const int CANVAS_X = canvas->core.x;
        const int CANVAS_Y = canvas->core.y;
        const int CANVAS_WIDTH = canvas->core.width;
        const int CANVAS_HEIGHT = canvas->core.height;

        if (x >= CANVAS_X && x <= CANVAS_X + CANVAS_WIDTH && y >= CANVAS_Y && y <= CANVAS_Y + CANVAS_HEIGHT)
        {
            active_backend->RedrawSprite(canvas->core.sprite);

            if (canvas->callback)
            {
                canvas->callback(x - CANVAS_X, y - CANVAS_Y);
            }
        }
    }
}

#endif