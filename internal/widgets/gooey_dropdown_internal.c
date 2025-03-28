#include "gooey_dropdown_internal.h"
#include "backends/gooey_backend_internal.h"

void GooeyDropdown_Draw(GooeyWindow *win)
{
    

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        int x_offset = win->dropdowns[i]->core.x;

        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown->core.is_visible)
            continue;
        active_backend->FillRectangle(dropdown->core.x,
                                      dropdown->core.y, dropdown->core.width,
                                      dropdown->core.height, win->active_theme->widget_base, win->creation_id);

        active_backend->DrawText(dropdown->core.x + 5,
                                 dropdown->core.y + 20,
                                 dropdown->options[dropdown->selected_index],
                                 win->active_theme->neutral, 0.25f, win->creation_id);
        int text_width = active_backend->GetTextWidth(dropdown->options[dropdown->selected_index], strlen(dropdown->options[dropdown->selected_index]));
        active_backend->DrawText(x_offset, 15,
                                 dropdown->options[dropdown->selected_index], win->active_theme->neutral, 0.25f, win->creation_id);
        if (dropdown->is_open && dropdown->num_options > 0)
        {
            int submenu_x = x_offset;
            int submenu_y = win->dropdowns[i]->core.y + win->dropdowns[i]->core.height;
            int submenu_width = win->dropdowns[i]->core.width;
            int submenu_height = 25 * dropdown->num_options;
            active_backend->FillRectangle(submenu_x, submenu_y,
                                          submenu_width, submenu_height, win->active_theme->widget_base, win->creation_id);
            active_backend->DrawRectangle(submenu_x, submenu_y,
                                          submenu_width, submenu_height, win->active_theme->primary, win->creation_id);
            for (int j = 0; j < dropdown->num_options; j++)
            {
                int element_y = submenu_y + (j * 25);
                active_backend->DrawText(submenu_x + 5,
                                         element_y + 18, dropdown->options[j], win->active_theme->neutral, 0.25f, win->creation_id);
                if (j < dropdown->num_options - 1)
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

bool GooeyDropdown_HandleClick(GooeyWindow *win, int x, int y)
{

    bool _btn_st = false;
    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        int x_offset = win->dropdowns[i]->core.x;
        GooeyDropdown *dropdown = win->dropdowns[i];
        int text_width = 10;
        if (x >= dropdown->core.x && x <= dropdown->core.x + dropdown->core.width && y >= dropdown->core.y && y <= dropdown->core.y + dropdown->core.height)
        {
            dropdown->is_open = !dropdown->is_open;

            _btn_st = true;
        }

        if (dropdown->is_open)
        {
            int submenu_x = x_offset;
            int submenu_y = win->dropdowns[i]->core.y + win->dropdowns[i]->core.height;
            int submenu_width = win->dropdowns[i]->core.width;
            for (int j = 0; j < dropdown->num_options; j++)
            {
                int element_y = submenu_y + (j * 25);
                if (x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= element_y && y <= element_y + 25)
                {
                    dropdown->selected_index = j;

                    if (win->dropdowns[i]->callback)
                        win->dropdowns[i]->callback(j);

                    dropdown->is_open = 0;
                    return true;
                }
            }
        }

        x_offset += text_width + 20;
    }
    if (_btn_st)
    {
        return true;
    }
    else
    {
        return false;
    }
}
