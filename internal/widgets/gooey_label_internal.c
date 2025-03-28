#include "gooey_label_internal.h"
#include "backends/gooey_backend_internal.h"

void GooeyLabel_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->label_count; ++i)
    {
        if (!win->labels[i]->core.is_visible)
            continue;
        active_backend->DrawText(win->labels[i]->core.x, win->labels[i]->core.y, win->labels[i]->text, win->labels[i]->color != (unsigned long)-1 ? win->labels[i]->color : win->active_theme->neutral, win->labels[i]->font_size, win->creation_id);
    }
}