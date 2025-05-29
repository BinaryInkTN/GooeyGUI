#include "gooey_button_internal.h"
#if (ENABLE_BUTTON==1)
#include "backends/gooey_backend_internal.h"

#define GOOEY_BUTTON_DEFAULT_RADIUS 2.0f

void GooeyButton_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        if (!button->core.is_visible)
            continue;

        unsigned long button_color = win->active_theme->widget_base;

        if (button->hover)
        {
            button_color = ((button_color & 0x7E7E7E) >> 1) | (button_color & 0x808080) >> 1; // A little darker
        }
        active_backend->FillRectangle(button->core.x,
                                      button->core.y, button->core.width, button->core.height, button_color, win->creation_id, true, GOOEY_BUTTON_DEFAULT_RADIUS);

        if (button->clicked)
            active_backend->DrawRectangle(button->core.x,
                                          button->core.y, button->core.width, button->core.height, win->active_theme->primary, 1.0f, win->creation_id, true, 4.0f);

        float text_width = active_backend->GetTextWidth(button->label, strlen(button->label));
        float text_height = active_backend->GetTextHeight(button->label, strlen(button->label));

        float text_x = button->core.x + (button->core.width - text_width) / 2;
        float text_y = button->core.y + (button->core.height + text_height) / 2;

        active_backend->DrawText(text_x,
                                 text_y, button->label, win->active_theme->neutral, 0.27f, win->creation_id);
        active_backend->SetForeground(win->active_theme->neutral);

        if (button->is_highlighted)
        {

            active_backend->DrawRectangle(button->core.x,
                                          button->core.y, button->core.width, button->core.height, win->active_theme->primary, 1.0f, win->creation_id, true, GOOEY_BUTTON_DEFAULT_RADIUS);
        }
    }
}
bool GooeyButton_HandleHover(GooeyWindow *win, int x, int y)
{
    static bool was_hovered = false;
    bool hover_over_button = false;

    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        if (!button || !button->core.is_visible) // Safety check
            continue;

        bool is_within_bounds = (x >= button->core.x && x <= button->core.x + button->core.width) &&
                                (y >= button->core.y && y <= button->core.y + button->core.height);
        button->hover = is_within_bounds;

        if (is_within_bounds) {
            hover_over_button = true;
            break;
        }
    }

    if (hover_over_button != was_hovered) {
        active_backend->CursorChange(hover_over_button ? GOOEY_CURSOR_HAND : GOOEY_CURSOR_ARROW);
        was_hovered = hover_over_button;
    }

    return hover_over_button;
}


bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y)
{
    bool clicked_any_button = false;

    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        if (!button || !button->core.is_visible)
            continue;
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
#endif