#include "gooey_button_internal.h"
#include "backends/gooey_backend_internal.h"


void GooeyButton_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        active_backend->FillRectangle(button->core.x,
                                      button->core.y, button->core.width, button->core.height, button->clicked ? win->active_theme->primary : win->active_theme->widget_base, win->creation_id);
        float text_width = active_backend->GetTextWidth(button->label, strlen(button->label));
        float text_height = active_backend->GetTextHeight(button->label, strlen(button->label));

        float text_x = button->core.x + (button->core.width - text_width) / 2;
        float text_y = button->core.y + (button->core.height + text_height) / 2;

        active_backend->DrawText(text_x,
                                 text_y, button->label, button->clicked ? win->active_theme->base : win->active_theme->neutral, 0.25f, win->creation_id);
        active_backend->SetForeground(win->active_theme->neutral);

        if (button->is_highlighted)
        {

            active_backend->DrawRectangle(button->core.x,
                                          button->core.y, button->core.width, button->core.height, win->active_theme->primary, win->creation_id);
        }
    }
}

bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y)
{
    bool clicked_any_button = false;

    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        bool is_within_bounds = (x >= button->core.x && x <= button->core.x + button->core.width) &&
                                (y >= button->core.y && y <= button->core.y + button->core.height);

        if (is_within_bounds)
        {
            button->clicked = !button->clicked;
            clicked_any_button = true;
            if (button->callback)
            {
                button->callback();
            }
        }
        else
        {
            button->clicked = false;
        }
    }

    return clicked_any_button;
}
