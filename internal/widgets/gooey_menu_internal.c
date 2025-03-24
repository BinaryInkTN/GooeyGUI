#include "gooey_menu_internal.h"
#include "backends/gooey_backend_internal.h"


void GooeyMenu_Draw(GooeyWindow *win)
{

    if (win->menu)
    {
        int window_width, window_height;

        active_backend->GetWinDim(&window_width, &window_height, win->creation_id);
        active_backend->FillRectangle(0, 0, window_width, 20, win->active_theme->widget_base, win->creation_id);

        int x_offset = 10;
        for (int i = 0; i < win->menu->children_count; i++)
        {
            GooeyMenuChild *child = &win->menu->children[i];
            int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));
            active_backend->DrawText(x_offset, 15,
                                     child->title, win->active_theme->neutral, 0.25f, win->creation_id);

            if (child->is_open && child->menu_elements_count > 0)
            {
                int submenu_x = x_offset - 10;
                int submenu_y = 20;
                int submenu_width = 150;
                int submenu_height = 25 * child->menu_elements_count;

                active_backend->FillRectangle(submenu_x, submenu_y,
                                              submenu_width, submenu_height, win->active_theme->widget_base, win->creation_id);

                for (int j = 0; j < child->menu_elements_count; j++)
                {
                    int element_y = submenu_y + (j * 25);
                    active_backend->DrawText(submenu_x + 5,
                                             element_y + 18, child->menu_elements[j], win->active_theme->neutral, 0.25f, win->creation_id);
                    if (j < child->menu_elements_count - 1)
                    {

                        active_backend->DrawLine(submenu_x,
                                                 element_y + 25 - 1, submenu_x + submenu_width,
                                                 element_y + 25 - 1, win->active_theme->neutral, win->creation_id);
                    }
                }
            }

            x_offset += text_width + 20;
        }
    }
}

bool GooeyMenu_HandleClick(GooeyWindow *win, int x, int y)
{

    if (!win->menu)
        return false;

    int x_offset = 10;
    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));
        if (y <= 20 && x >= x_offset && x <= x_offset + text_width)
        {

            for (int k = 0; k < win->menu->children_count; k++)
            {
                win->menu->children[k].is_open = 0;
            }

            child->is_open = !child->is_open;
            win->menu->is_busy = !win->menu->is_busy;

            
            return true;
        }

        if (child->is_open)
        {

            int submenu_x = x_offset - 10;
            int submenu_y = 20;
            int submenu_width = 150;

            for (int j = 0; j < child->menu_elements_count; j++)
            {
                int element_y = submenu_y + (j * 25);
                if (x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= element_y && y <= element_y + 25)
                {

                    if (child->callbacks[j])
                        child->callbacks[j]();

                    for (int k = 0; k < win->menu->children_count; k++)
                    {
                        win->menu->children[k].is_open = 0;
                    }
                    win->menu->is_busy = 0;

                    
                    return true;
                } else {
                    for (int k = 0; k < win->menu->children_count; k++)
                    {
                        win->menu->children[k].is_open = 0;
                        win->menu->is_busy = 0;
                    }
                    
                    return true;
                }
            }
        }

        x_offset += text_width + 20;
    }
    return false;
}
