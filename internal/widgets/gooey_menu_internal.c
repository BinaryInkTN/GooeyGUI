#include "gooey_menu_internal.h"
#if(ENABLE_MENU)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"

void GooeyMenu_Draw(GooeyWindow *win)
{
    if (!win->menu)
        return;

    int window_width, window_height;
    active_backend->GetWinDim(&window_width, &window_height, win->creation_id);

    active_backend->FillRectangle(0, 0, window_width, 20, win->active_theme->widget_base, win->creation_id, false, 0.0f);

    int x_offset = 10;
    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));

        if (child->is_open)
        {
            active_backend->FillRectangle(x_offset - 10, 0, text_width + 20, 20,
                                          win->active_theme->primary, win->creation_id, false, 0.0f);
        }

        active_backend->DrawText(x_offset, 15, child->title,
                                 child->is_open ? win->active_theme->base : win->active_theme->neutral,
                                 0.27f, win->creation_id);

        if (child->is_open && child->menu_elements_count > 0)
        {
            const int submenu_x = x_offset - 10;
            const int submenu_y = 20;
            const int submenu_width = 150;
            const int submenu_height = 25 * child->menu_elements_count;

            active_backend->FillRectangle(submenu_x, submenu_y, submenu_width, submenu_height,
                                          win->active_theme->widget_base, win->creation_id, true, 5.0f);

            for (int j = 0; j < child->menu_elements_count; j++)
            {
                const int element_y = submenu_y + (j * 25);
                const bool is_hovered = (child->element_hovered_over == j);

                if (is_hovered)
                {
                    active_backend->FillRectangle(submenu_x, element_y, submenu_width, 25,
                                                  win->active_theme->primary, win->creation_id, true, 2.0f);
                }

                active_backend->DrawText(submenu_x + 5, element_y + 18, child->menu_elements[j],
                                         is_hovered ? win->active_theme->base : win->active_theme->neutral,
                                         0.27f, win->creation_id);

                if (j < child->menu_elements_count - 1)
                {
                    active_backend->DrawLine(submenu_x, element_y + 24,
                                             submenu_x + submenu_width, element_y + 24,
                                             win->active_theme->neutral, win->creation_id);
                }
            }
        }
        else
        {
            child->element_hovered_over = -1;
        }

        x_offset += text_width + 20;
    }
}

bool GooeyMenu_HandleHover(GooeyWindow *win)
{
    if (!win->menu)
        return false;

    GooeyEvent *event = (GooeyEvent *)win->current_event;
    const int x = event->mouse_move.x;
    const int y = event->mouse_move.y;
    bool is_any_element_hovered_over = false;

    int x_offset = 10;
    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];

        if (child->is_open)
        {
            const int submenu_x = x_offset - 10;
            const int submenu_y = 20;
            const int submenu_width = 150;

            for (int j = 0; j < child->menu_elements_count; j++)
            {
                const int element_y = submenu_y + (j * 25);

                if (x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= element_y && y <= element_y + 25)
                {
                    child->element_hovered_over = j;
                    is_any_element_hovered_over = true;
                }
            }
        }
        x_offset += active_backend->GetTextWidth(child->title, strlen(child->title)) + 20;
    }

    return is_any_element_hovered_over;
}

bool GooeyMenu_HandleClick(GooeyWindow *win, int x, int y)
{
    if (!win->menu)
        return false;

    bool handled = false;
    int x_offset = 10;

    for (int i = 0; i < win->menu->children_count && !handled; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        const int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));

        if (y <= 20 && x >= x_offset && x <= x_offset + text_width)
        {

            for (int k = 0; k < win->menu->children_count; k++)
            {
                win->menu->children[k].is_open = (k == i) ? !win->menu->children[k].is_open : 0;
            }
            win->menu->is_busy = !win->menu->is_busy;
            handled = true;
            break;
        }

        if (child->is_open)
        {
            const int submenu_x = x_offset - 10;
            const int submenu_y = 20;
            const int submenu_width = 150;

            for (int j = 0; j < child->menu_elements_count && !handled; j++)
            {
                const int element_y = submenu_y + (j * 25);

                if (x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= element_y && y <= element_y + 25)
                {

                    if (child->callbacks[j])
                    {
                        child->callbacks[j]();
                    }
                    handled = true;
                }
            }

            if (handled ||
                (x < submenu_x || x > submenu_x + submenu_width ||
                 y < submenu_y || y > submenu_y + (25 * child->menu_elements_count)))
            {
                for (int k = 0; k < win->menu->children_count; k++)
                {
                    win->menu->children[k].is_open = 0;
                }
                win->menu->is_busy = 0;
                return true;
            }
        }

        x_offset += text_width + 20;
    }

    return handled;
}
#endif