#include "widgets/gooey_button_internal.h"
#if (ENABLE_BUTTON)
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

        if (button->is_disabled || button->hover)
        {
            button_color = ((button_color & 0x7E7E7E) >> 1) | (button_color & 0x808080) >> 1; // A little darker
        }

         active_backend->FillRectangle(button->core.x+1, button->core.y+1, button->core.width-2, button->core.height-2, button_color, win->creation_id, true, GOOEY_BUTTON_DEFAULT_RADIUS, button->core.sprite);


        float text_width = active_backend->GetTextWidth(button->label, strlen(button->label));
        float text_height = active_backend->GetTextHeight(button->label, strlen(button->label));

        float text_x = button->core.x + (button->core.width - text_width) / 2;
        float text_y = button->core.y + (button->core.height + text_height) / 2;

        active_backend->DrawGooeyText(text_x,
                                 text_y, button->label, win->active_theme->neutral, 18.0f, win->creation_id, button->core.sprite);
        active_backend->SetForeground(win->active_theme->neutral);



        if (button->core.sprite && button->core.sprite->needs_redraw)
            active_backend->ResetRedrawSprite(button->core.sprite);
    }
}

bool GooeyButton_HandleHover(GooeyWindow *win, int x, int y)
{
    static bool was_hovered = false;
    bool hover_over_button = false;
    bool forced_redraw = false;
    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        if (!button || button->is_disabled || !button->core.is_visible) // Safety check
            continue;

        bool is_within_bounds = (x >= button->core.x && x <= button->core.x + button->core.width) &&
                                (y >= button->core.y && y <= button->core.y + button->core.height);
        button->hover = is_within_bounds;

        if (is_within_bounds && !button->core.disable_input)
        {
            hover_over_button = true;
            break;
        } else {
            button->hover = false;
            forced_redraw = was_hovered;
        }
    }
    if (hover_over_button != was_hovered)
    {
        active_backend->CursorChange(hover_over_button ? GOOEY_CURSOR_HAND : GOOEY_CURSOR_ARROW);
        was_hovered = hover_over_button;
    }
    return hover_over_button || forced_redraw;
}

bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y)
{
    bool clicked_any_button = false;

    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        if (!button || button->is_disabled || !button->core.is_visible || button->core.disable_input)
            continue;
        bool is_within_bounds = (x >= button->core.x && x <= button->core.x + button->core.width) &&
                                (y >= button->core.y && y <= button->core.y + button->core.height);
        if (is_within_bounds && !button->core.disable_input)
        {

            button->clicked = !button->clicked;
            clicked_any_button = true;
            active_backend->RedrawSprite(button->core.sprite);

            if (!button->is_disabled && button->callback)
            {
                button->callback(button->user_data);
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
