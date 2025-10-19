#include "widgets/gooey_dropdown_internal.h"
#if (ENABLE_DROPDOWN)
#include "backends/gooey_backend_internal.h"

void GooeyDropdown_Draw(GooeyWindow *win)
{
    if (!win || !win->dropdowns)
        return;

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown || !dropdown->core.is_visible)
            continue;

        active_backend->FillRectangle(dropdown->core.x, dropdown->core.y,
                                      dropdown->core.width, dropdown->core.height,
                                      win->active_theme->widget_base, win->creation_id, true, 2.0f, dropdown->core.sprite);

        const char *selected_text = dropdown->options[dropdown->selected_index];
        active_backend->DrawText(dropdown->core.x + 5, dropdown->core.y + 20,
                                 selected_text, win->active_theme->neutral,
                                 0.27f, win->creation_id, dropdown->core.sprite);

        if (dropdown->is_open && dropdown->num_options > 0)
        {
            const int submenu_x = dropdown->core.x;
            const int submenu_y = dropdown->core.y + dropdown->core.height;
            const int submenu_width = dropdown->core.width;
            const int submenu_height = 25 * dropdown->num_options;

            active_backend->FillRectangle(submenu_x, submenu_y,
                                          submenu_width, submenu_height,
                                          win->active_theme->widget_base, win->creation_id, true, 2.0f, dropdown->core.sprite);
            active_backend->DrawRectangle(submenu_x, submenu_y,
                                          submenu_width, submenu_height,
                                          win->active_theme->primary, 0.5f, win->creation_id, true, 2.0f, dropdown->core.sprite);

            for (int j = 0; j < dropdown->num_options; j++)
            {
                const int element_y = submenu_y + (j * 25);
                const bool is_hovered = (dropdown->element_hovered_over == j);
                const bool is_selected = (j == dropdown->selected_index);

                if (is_hovered || is_selected)
                {
                    active_backend->FillRectangle(submenu_x, element_y,
                                                  submenu_width, 25,
                                                  win->active_theme->primary, win->creation_id, false, 0.0f, dropdown->core.sprite);
                }

                active_backend->DrawText(submenu_x + 5, element_y + 18,
                                         dropdown->options[j],
                                         (is_hovered || is_selected) ? win->active_theme->base : win->active_theme->neutral,
                                         0.27f, win->creation_id, dropdown->core.sprite);

                if (j < dropdown->num_options - 1)
                {
                    active_backend->DrawLine(submenu_x, element_y + 24,
                                             submenu_x + submenu_width, element_y + 24,
                                             win->active_theme->neutral, win->creation_id, dropdown->core.sprite);
                }
            }
        }
        if (dropdown->core.sprite && dropdown->core.sprite->needs_redraw)
            active_backend->ResetRedrawSprite(dropdown->core.sprite);
    }
}

bool GooeyDropdown_HandleHover(GooeyWindow *win, int x, int y)
{
    if (!win || !win->dropdowns)
        return false;

    bool hover_handled = false;

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown || !dropdown->core.is_visible || dropdown->core.disable_input)
            continue;

        if (x >= dropdown->core.x && x <= dropdown->core.x + dropdown->core.width &&
            y >= dropdown->core.y && y <= dropdown->core.y + dropdown->core.height)
        {
            hover_handled = true;
            continue;
        }

        if (dropdown->is_open)
        {
            const int submenu_x = dropdown->core.x;
            const int submenu_y = dropdown->core.y + dropdown->core.height;
            const int submenu_width = dropdown->core.width;

            dropdown->element_hovered_over = -1;

            for (int j = 0; j < dropdown->num_options; j++)
            {
                const int element_y = submenu_y + (j * 25);
                if (x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= element_y && y <= element_y + 25)
                {
                    dropdown->element_hovered_over = j;
                    hover_handled = true;
                    break;
                }
            }
        }
        else
        {
            dropdown->element_hovered_over = -1;
        }
    }

    return hover_handled;
}

bool GooeyDropdown_HandleClick(GooeyWindow *win, int x, int y)
{
    if (!win || !win->dropdowns)
        return false;

    bool click_handled = false;

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown || !dropdown->core.is_visible || dropdown->core.disable_input)
            continue;

        if (x >= dropdown->core.x && x <= dropdown->core.x + dropdown->core.width &&
            y >= dropdown->core.y && y <= dropdown->core.y + dropdown->core.height)
        {
            active_backend->RedrawSprite(dropdown->core.sprite);

            dropdown->is_open = !dropdown->is_open;
            click_handled = true;
            continue;
        }

        if (dropdown->is_open)
        {
            const int submenu_x = dropdown->core.x;
            const int submenu_y = dropdown->core.y + dropdown->core.height;
            const int submenu_width = dropdown->core.width;

            for (int j = 0; j < dropdown->num_options; j++)
            {
                const int element_y = submenu_y + (j * 25);
                if (x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= element_y && y <= element_y + 25)
                {
                    active_backend->RedrawSprite(dropdown->core.sprite);

                    dropdown->selected_index = j;
                    if (dropdown->callback)
                    {
                        dropdown->callback(j, dropdown->user_data);
                    }
                    click_handled = true;
                    break;
                }
            }

            dropdown->is_open = false;
        }
    }

    return click_handled;
}
#endif
